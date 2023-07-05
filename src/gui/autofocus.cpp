#include "newautofocus.h"

#include <fcntl.h>
#include <opencv2/opencv.hpp>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <qtlab/core/logger.h>

#include <QPixmap>

#include <thread>

#include "rapid-af.h"

using namespace cv;

static Logger *logger = getLogger("Autofocus");

Autofocus::Autofocus(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<CAlkUSB3::BufferPtr>("CAlkUSB3::BufferPtr");
}

void Autofocus::init()
{
    ICeleraCamera& dev1 = CAlkUSB3::ICeleraCamera::Create() ;
    ICeleraCamera& dev2 = CAlkUSB3::ICeleraCamera::Create() ;
    
    auto stringArray = dev1.GetCameraList();
    size_t n = stringArray.Size();

    if (n == 0) {
        throw std::runtime_error("Cannot find Alkeria camera.");
    }

    logger->info(QString("Found %1 Alkeria cameras").arg(n));

    for (size_t i = 0; i < n; ++i) {
        logger->info(QString(stringArray[i]));
    }

    dev = [dev1,dev2];

    virtual bool GetBandwidthLimitsAvailable ( );  //this is necessary when we have multiple cameras
    const Array< unsigned int >& bandwidthLimits;
    if (GetBandwidthLimitsAvailable()) {
            bandwidthLimits = GetAllowedBandwidthLimits();
    else {
        bandwidthLimits = new uint[] {32}; 
        }

    for (i=0; i<n; i++){
        try {
            dev[i].SetCamera(i);            // Open device
            dev[i].SetPreserveRates(false); // Ask always for the best performances (FPS)
            dev[i].SetColorCoding(CAlkUSB3::ColorCoding::Mono8);
            dev[i].SetEnableImageThread(false);
    
            dev[i].SetHorizontalBinning(2);
            dev[i].SetVerticalBinning(2);
        } catch (CAlkUSB3::Exception e) {
            logger->warning(e.Message());}

        dev[i].SetBandwidthLimits(bandwidthLimits);
        
        dev[i].RawFrameAcquired().SetUserData(this); // ??
        dev[i].RawFrameAcquired().SetCallbackEx(&Autofocus::onFrameAcquired); // ??
    
        dev[i].FrameStartTrigger.Source = TriggerSource.External;
        dev[i].FrameStartTrigger.ExternalInput = 1; // Trigger source is port 1
        dev[i].FrameStartTrigger.DetectExternalInputEdge = true;
        dev[i].FrameStartTrigger.InvertExternalInput = true; // Detect falling edge
        dev[i].FrameStartTrigger.Enabled = true; // Enable frame trigger (no free-run)
    }

void Autofocus::start()
{
    if (!enabled) {
        return;
    }
    if (dev[0].GetAcquire() && dev[1].GetAcquire() ) {
        stop();
    }

    thread startAcquire[2];
    
    auto start1([=]){
        try {
            dev[0].SetAcquire(true);
      } catch (CAlkUSB3::InvalidOperationException e) {
            throw std::runtime_error("Alkeria: invalid operation");}
    }
    
    auto start2([=]){
        try {
            dev[1].SetAcquire(true);
      } catch (CAlkUSB3::InvalidOperationException e) {
            throw std::runtime_error("Alkeria: invalid operation");}
    }
    
    startAcquire[0]=thread(acquire1);
    startAcquire[1]=thread(acquire2);
        
    if (opt.multithreading_enable) {
        for (int i = 0; i < 2; ++i) {
            startAcquire[i].join();
        }

        if (teptr) {
            std::rethrow_exception(teptr);
        }
    } 
}

void Autofocus::stop()
{    
    thread stopAcquire[2];

    auto stop1 ([=]){dev[0].SetAcquire(false)};
    auto stop2 ([=]){dev[1].SetAcquire(false)};
    stopAcquire[0] = thread(stop1);
    stopAcquire[1] = thread(stop2);
    if (opt.multithreading_enable) {
        for (int i = 0; i < 2; ++i) {
            stopAcquire[i].join();
        }

    if (teptr) {
            std::rethrow_exception(teptr);
        }
    } 
}

/**
 * @brief This callback is called upon frame acquisition.
 * @param userData A pointer to this class object.
 */

void Autofocus::onFrameAcquired(void *userData)
{
    if (!userData)
        return;

    Autofocus af = static_cast<Autofocus *>(userData);

    thread videoStream[2];

    auto video1([&]){
            CAlkUSB3::IVideoSource &videoSource1(dev[0]);
            CAlkUSB3::BufferPtr ptr1 = videoSource1.GetRawDataPtr(false);
    }
    auto video2([&]){
            CAlkUSB3::IVideoSource &videoSource2(dev[1]);
            CAlkUSB3::BufferPtr ptr2 = videoSource2.GetRawDataPtr(false);
    }

    videoStream[0] = thread(video1);
    videoStream[1] = thread(video2);
    videoStream[0].join();
    videoStream[1].join();

    QList<CAlkUSB3::BufferPtr> ptr = {ptr1,ptr2};
    emit af->newImage(ptr);
    
    QList<double> deltaList;
    
    try {
        deltaList = af->getDelta();
    } catch (std::runtime_error e) {
        emit af->newStatus(e.what());
        return;
    }
    double averageDelta = (deltaList[0] + deltaList[1] + deltaList[2] + deltaList[3])/deltaList.Size();

    double alpha = af->mAlpha *averageDelta + af->qAlpha;
    double beta1 = af->mbeta1 *(deltaList[1]-deltaList[0] + deltaList[3]-deltaList[2])/2 + af->qbeta1;
    double beta2 = af->mbeta2 *(deltaList[0]-deltaList[2] + deltaList[1]-deltaList[3])/2 + af->qbeta2;
    correctionList = {alpha, beta1, beta2};
        
    if (af->isOutputEnabled()) {
        emit af->newCorrection(correctionList);
    }
    emit af->newStatus(QString("dx = %1, corr = %2").arg(deltaList).arg(correctionList));
}

QList<double> Autofocus::getDelta()
{
    auto video1([&]){
            CAlkUSB3::IVideoSource &videoSource1(dev[0]);
            CAlkUSB3::BufferPtr ptr1 = videoSource1.GetRawDataPtr(false);
    }
    auto video2([&]){
            CAlkUSB3::IVideoSource &videoSource2(dev[1]);
            CAlkUSB3::BufferPtr ptr2 = videoSource2.GetRawDataPtr(false);
    }

    videoStream[0] = thread(video1);
    videoStream[1] = thread(video2);
    videoStream[0].join();
    videoStream[1].join();

    if (!ptr1 || !ptr2) {
        throw std::runtime_error("No frame was received");
    }

    cd_height = ptr1.GetHeight();  // ptr1 and ptr2 will have same dimensions so...
    cd_width = ptr1.GetWidth();
        
    image1= Mat(cd_height, cd_width, CV_8U, (void *) ptr1.Data());
    image2= Mat(cd_height, cd_width, CV_8U, (void *) ptr2.Data()); 

    int x1 = static_cast<int>(cd_width*0.05);  // 5% of img_width
    int x2 = static_cast<int>(cd_width*0.6);   // 60% of img_width
    int y1 = static_cast<int>(cd_height*0.6);  // 60% of img_height
    int y2 = static_cast<int>(cd_height*0.05); // 5% of img_height
    
    int roi_width = static_cast<int>(cd_width*0.4);
    int roi_height = static_cast<int>(cd_height*0.4);

    cv::Rect roi1(x1, y1, roi_width, roi_height); // upperLeft ROI
    cv::Rect roi2(x2, y1, roi_width, roi_height); // upperRight ROI
    cv::Rect roi3(x1, y2, roi_width, roi_height); // buttomLeft ROI
    cv::Rect roi4(x2, y2, roi_width, roi_height); // buttomRight ROI

    roi = {roi1, roi2, roi3, roi4}

    QList<QList<cv::Mat>> couple = {
        {image1(roi[0]), image2(roi[0])},  // couple1
        {image1(roi[1]), image2(roi[1])},  // couple2
        {image1(roi[2]), image2(roi[2])},  // couple3
        {image1(roi[3]), image2(roi[3])}   // couple4
    };

    if (imageQualityEnabled) {
        if (!rapid_af::checkImageQuality(couple[0][0], iqOptions)
            || !rapid_af::checkImageQuality(couple[0][1], iqOptions)
            || !rapid_af::checkImageQuality(couple[1][0], iqOptions)
            || !rapid_af::checkImageQuality(couple[1][1], iqOptions)
            || !rapid_af::checkImageQuality(couple[2][0], iqOptions)
            || !rapid_af::checkImageQuality(couple[2][1], iqOptions)
            || !rapid_af::checkImageQuality(couple[3][0], iqOptions)
            || !rapid_af::checkImageQuality(couple[3][1], iqOptions)) {
            throw std::runtime_error("low image quality");
        }
    }

    thread myThreads[4];
    bool ok = false;

    Point2f shift;
    QList<double> deltaList;

    static std::exception_ptr teptr = nullptr;

    double alignThem([&](int j))
{
        try {
        shift = rapid_af::align(couple[j][0], couple[j][1], options, &ok);
    }   catch (cv::Exception e) {
            logger->warning(e.what());
            throw std::runtime_error("OpenCV exception (see log)");
        }
        if (!ok) {
            throw std::runtime_error("Agreement threshold not met");   
        }
        else {
        deltaList.append(shift.x);
        shiftList.append(shift); //to be used in getMergedImage()
        }
}
    for (j=0; j<4; j++){
        myThreads[j] = thread(alignThem, j);
        ok = false;
    }

    if (opt.multithreading_enable) {
        for (int i = 0; i < 4; ++i) {
            myThreads[i].join();
        }

        if (teptr) {
            std::rethrow_exception(teptr);
        }
    }

    return deltaList;
}

double Autofocus::inferCalibrationQAlpha()
{    
    qAlpha = -mAlpha* correctionList[0];
    return qAlpha;
}

double Autofocus::inferCalibrationQBeta1()
{    
    qBeta1 = -mBeta1* correctionList[1];
    return qBeta1;
}

double Autofocus::inferCalibrationQBeta2()
{    
    qBeta2 = -mBeta2* correctionList[2];
    return qBeta2;
}

rapid_af::AlignOptions Autofocus::getOptions() const
{
    return options;
}

void Autofocus::setOptions(const rapid_af::AlignOptions &value)
{
    options = value;
}

rapid_af::ImageQualityOptions Autofocus::getIqOptions() const
{
    return iqOptions;
}

void Autofocus::setIqOptions(const rapid_af::ImageQualityOptions &value)
{
    iqOptions = value;
}

bool Autofocus::isImageQualityEnabled() const
{
    return imageQualityEnabled;
}

void Autofocus::setImageQualityEnabled(bool enable)
{
    imageQualityEnabled = enable;
}

double Autofocus::getExposureTime_us() const
{
    return exposureTime_us;
}

void Autofocus::setExposureTime_us(double value)
{
    exposureTime_us = value;
}

double Autofocus::getFrameRate() const
{
    return frameRate;
}

void Autofocus::setFrameRate(double value)
{
    frameRate = value;
}

double Autofocus::getCalibration_mAlpha() const
{
    return mAlpha;
}
void Autofocus::setCalibrationAlpha(double mAlpha, double qAlpha)
{
    this->mAlpha = mAlpha;
    this->qAlpha = qAlpha;
}
QList<double> Autofocus::getCalibration_qAlpha() const
{     
    return qAlpha;
}

double Autofocus::getCalibration_mBeta1() const
{
    return mBeta1;
}
void Autofocus::setCalibrationBeta1(double mBeta1, double qBeta1)
{
    this->mBeta1 = mBeta1;
    this->qBeta1 = qBeta1;
}
QList<double> Autofocus::getCalibration_qBeta1() const
{     
    return qBeta1;
}

double Autofocus::getCalibration_mBeta2() const
{
    return mBeta2;
}
void Autofocus::setCalibrationBeta2(double mBeta2, double qBeta2)
{
    this->mBeta2 = mBeta2;
    this->qBeta2 = qBeta2;
}
QList<double> Autofocus::getCalibration_qBeta2() const
{     
    return qBeta2;
}

bool Autofocus::isEnabled() const
{
    return enabled;
}

void Autofocus::setEnabled(bool enable)
{
    enabled = enable;
}

bool Autofocus::isOutputEnabled() const
{
    return outputEnabled;
}

void Autofocus::setOutputEnabled(bool enable)
{
    outputEnabled = enable;
}

cv::Mat Autofocus::getImage1() const
{
    return image1;
}

cv::Mat Autofocus::getImage2() const
{
    return image2;
}


QList<QImage> Autofocus::getMergedImage()
{     
    QList<Mat> mergedImages;
    Mat img1;
    int width = qMin(roi[0].width(), roi[0].width()) - 1;
    int height = qMin(roi[0].height(), roi[0].height()) - 1;   // All ROIs have same dimensions
    try {
        img1 = rapid_af::merge(couple1[0](Range(0, height), Range(0, width)),
                              couple1[1](Range(0, height), Range(0, width)),
                              shiftList[0]);
    } catch (cv::Exception e) {
    };
        
    Mat rgb;
    cv::cvtColor(img1, rgb, cv::COLOR_BGR2RGB);
    QImage qimg1 = QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
    mergedImages.append(qimg1);
        
    Mat img2;
    try {
        img2 = rapid_af::merge(couple2[0](Range(0, height), Range(0, width)),
                              couple2[1](Range(0, height), Range(0, width)),
                              shiftList[1]);
    } catch (cv::Exception e) {
    };

    cv::cvtColor(img2, rgb, cv::COLOR_BGR2RGB);
    QImage qimg2 = QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
    mergedImages.append(qimg2);

    Mat img3;
    try {
        img3 = rapid_af::merge(couple3[0](Range(0, height), Range(0, width)),
                              couple3[1](Range(0, height), Range(0, width)),
                              shiftList[2]);
    } catch (cv::Exception e) {
    };

    cv::cvtColor(img3, rgb, cv::COLOR_BGR2RGB);
    QImage qimg3 = QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
    mergedImages.append(qimg3);

    Mat img4;
    try {
        img4 = rapid_af::merge(couple4[0](Range(0, height), Range(0, width)),
                              couple4[1](Range(0, height), Range(0, width)),
                              shiftList[3]);
    } catch (cv::Exception e) {
    };

    cv::cvtColor(img4, rgb, cv::COLOR_BGR2RGB);
    QImage qimg4 = QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
    mergedImages.append(qimg4);

    return mergedImages;
}
