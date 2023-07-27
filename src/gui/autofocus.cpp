#include "autofocus.h"

#include <fcntl.h>
#include <opencv2/opencv.hpp>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <qtlab/core/logger.h>

#include <QPixmap>

#include <QList>
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
    dev1 = CAlkUSB3::ICeleraCamera::Create() ; //two camera devices are created
    dev2 = CAlkUSB3::ICeleraCamera::Create() ;
    
    auto stringArray = dev1.GetCameraList();  // each contain both cameras
    n = stringArray.Size();

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
            bandwidthLimits = GetAllowedBandwidthLimits();   //gets a list of bandwidth limits for each usb port
            double n = bandwidthLimits.size();
            bandwidthLimits = bandwidthLimits[n/2];   //sets it to some value inside the middle of the list
    }
    else {
        bandwidthLimits = new uint[] {24};            //else, sets it to 24 KiB per micro-frame for each usb port
        }

    taskHandle = 0;
    data = 1;      // initiating the trigger to high (on)
    DAQmxCreateTask("Trigger", &taskHandle);    
    DAQmxCreateDOChan(taskHandle,"dev1/port1/line0:1"); // Creating two digital channels
    int32 written;
    DAQmxStartTask(taskHandle);
    DAQmxWriteDigitalU8(taskHandle, 1, 1, 10.0, DAQmx_Val_GroupByChannel, &data, &written, NULL);  // generating a digital signal
    DAQmxStopTask(taskHandle);
    DAQmxClearTask(taskHandle);

    for (i=0; i<n; i++){
        try {
            dev[i].SetCamera(i);            // Each dev opens a camera
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

        dev[i].PIOPorts[4].Direction = IODirection.Input; // if this is a bidirectional port, sets it to input
        dev[i].PIOPorts[4].DebounceTime = 10; // Set debounce time to 10us
        dev[i].PIOPorts[4].Termination = true; // Enable termination
        bool status = dev[i].PIOPorts[4].Value; // Read port 4 value
        rising = dev[i].PIOPorts[4].RisingEvent; // Read port 4 rising event
        //bool falling = device.PIOPorts[4].FallingEvent;
        dev[i].SetAcquire(rising);
    }
}

void Autofocus::triggerAcquisition(){
    for (i=0; i<n; i++){  //for both cameras
        if (data ==1){
            dev[i].SetAcquire(true);}  //video acqisition on
        else{
            dev[i].SetAcquire(false);  //video acqisition off
        }
    }
}

void Autofocus::start(){
    if (!enabled) {
        return;
    }
    if (dev[0].GetAcquire() && dev[1].GetAcquire() ) {
        stop();
    }
    DAQmxStartTask(taskHandle);
    data = 1;      //set to high

    for (i=0; i<n; i++){  
        dev[i].SetEnableImageThread(false); // No image conversion needed
        dev[i].SetFrameRate(frameRate);
        dev[i].SetShutter(exposureTime_us);}

    try {
        triggerAcquisition(); // getAcquire() becomes true
    } catch (CAlkUSB3::InvalidOperationException e) {
        throw std::runtime_error("Alkeria: invalid operation");
    }
    
    DAQmxStopTask(taskHandle);
    DAQmxClearTask(taskHandle);
}

void Autofocus::stop() {
    DAQmxStartTask(taskHandle);
    data=0;
    triggerAcquisition();
    DAQmxStopTask(taskHandle);
    DAQmxClearTask(taskHandle);
}

/**
 * @brief This callback is called upon frame acquisition.
 * @param userData A pointer to this class object.
 */

void Autofocus::onFrameAcquired(void *userData){
    if (!userData)
        return;

    Autofocus af = static_cast<Autofocus *>(userData);

    thread videoStream[2];   //stream both simultaneously using thread

    QList<CAlkUSB3::BufferPtr> ptr; //create a ptr list
    CAlkUSB3::BufferPtr ptr1;
    CAlkUSB3::BufferPtr ptr2;

    auto video1 = [&] (){
            CAlkUSB3::IVideoSource &videoSource1(af->dev[0]);
            ptr1 = videoSource1.GetRawDataPtr(false);
            ptr.append(ptr1);
    };
    auto video2 = [&] (){
            CAlkUSB3::IVideoSource &videoSource2(af->dev[1]);
            ptr2 = videoSource2.GetRawDataPtr(false);
            ptr.append(ptr2);
    };

    videoStream[0] = thread(video1);
    videoStream[1] = thread(video2);
    videoStream[0].join();
    videoStream[1].join();

    emit af->newImage(ptr);  //newImage returns a QList of ptrs
    
    QList<double> deltaList;
    
    try {
        deltaList = af->getDelta();
    } catch (std::runtime_error e) {
        emit af->newStatus(e.what());
        return;
    }
    double averageDelta = (deltaList[0] + deltaList[1] + deltaList[2] + deltaList[3])/deltaList.size();  //for alpha
    double deltaB1 = (deltaList[1]-deltaList[0] + deltaList[3]-deltaList[2])/2;  //for beta1
    double deltaB2 = (deltaList[0]-deltaList[2] + deltaList[1]-deltaList[3])/2; //for beta2
        
    double alpha = af->mAlpha *averageDelta + af->qAlpha;
    double beta1 = af->mbeta1 *deltaB1 + af->qbeta1;
    double beta2 = af->mbeta2 *deltaB2 + af->qbeta2;
    correctionList = {alpha, beta1, beta2};  
        
    if (af->isOutputEnabled()) {
        emit af->newCorrection(correctionList); //This is the correction signal generated
    }
    emit af->newStatus(QString("dx = %1, corr = %2").arg(averageDelta).arg(alpha));
    emit af->newStatus(QString("dx = %1, corr = %2").arg(deltaB1).arg(beta1));
    emit af->newStatus(QString("dx = %1, corr = %2").arg(deltaB2).arg(beta2));
}

QList<double> Autofocus::getDelta(){

    thread videoStream[2];

    QList<CAlkUSB3::BufferPtr> ptr; //create a ptr list
    CAlkUSB3::BufferPtr ptr1;
    CAlkUSB3::BufferPtr ptr2;
    
    auto video1([&]){
            CAlkUSB3::IVideoSource &videoSource1(dev[0]);
            ptr1 = videoSource1.GetRawDataPtr(false);
            ptr.append(ptr1);
    };
    auto video2([&]){
            CAlkUSB3::IVideoSource &videoSource2(dev[1]);
            ptr2 = videoSource2.GetRawDataPtr(false);
            ptr.append(ptr2);
    };

    videoStream[0] = thread(video1);
    videoStream[1] = thread(video2);
    videoStream[0].join();
    videoStream[1].join();

    if (!ptr1 || !ptr2) {
        throw std::runtime_error("No frame was received");
    }

    cd_height = ptr1.GetHeight();  // ptr1 and ptr2 will have same dimensions so...
    cd_width = ptr1.GetWidth();
        
    cv::Mat image1= Mat(cd_height, cd_width, CV_8U, (void *) ptr1.Data());
    cv::Mat image2= Mat(cd_height, cd_width, CV_8U, (void *) ptr2.Data()); 

    int x1 = static_cast<int>(cd_width*0.05);  // 5% of img_width
    int x2 = static_cast<int>(cd_width*0.55);   // 55% of img_width
    int y1 = static_cast<int>(cd_height*0.55);  // 55% of img_height
    int y2 = static_cast<int>(cd_height*0.05); // 5% of img_height
    
    int roi_width = static_cast<int>(cd_width*0.4);
    int roi_height = static_cast<int>(cd_height*0.4);

    cv::Rect roi1(x1, y1, roi_width, roi_height); // upperLeft ROI
    cv::Rect roi2(x2, y1, roi_width, roi_height); // upperRight ROI
    cv::Rect roi3(x1, y2, roi_width, roi_height); // buttomLeft ROI
    cv::Rect roi4(x2, y2, roi_width, roi_height); // buttomRight ROI

    roi = {roi1, roi2, roi3, roi4};

    QList<QList<cv::Mat>> couple = {
        {image1(roi[0]), image2(roi[0])},  // couple1
        {image1(roi[1]), image2(roi[1])},  // couple2
        {image1(roi[2]), image2(roi[2])},  // couple3
        {image1(roi[3]), image2(roi[3])}   // couple4
    }

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

    double alignThem([&](int j)){
        try {
        shift = rapid_af::align(couple[j][0], couple[j][1], options, &ok);   
        } catch (cv::Exception e) {
            logger->warning(e.what());
            throw std::runtime_error("OpenCV exception (see log)");
        }
        if (!ok) {
            throw std::runtime_error("Agreement threshold not met");   
        }
        else {
        deltaList.append(shift.x);  //to be used for calculating correction
        shiftList.append(shift); //anto be used in getMergedImage()}
    }}
    for (int j=0; j<4; j++){
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

void Autofocus::setImage1(const cv::Mat &value)
{
    image1 = value;
}

cv::Mat Autofocus::getImage2() const
{
    return image2;
}

void Autofocus::setImage2(const cv::Mat &value)
{
    image2 = value;
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
