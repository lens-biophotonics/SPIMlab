#include "newautofocus.h"

#include <fcntl.h>
#include <opencv2/opencv.hpp>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <qtlab/core/logger.h>

#include <QPixmap>

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

    auto stringArray = dev1.GetCameraList();

    size_t n = stringArray.Size();

    if (n == 0) {
        throw std::runtime_error("Cannot find Alkeria camera.");
    }

    logger->info(QString("Found %1 Alkeria cameras").arg(n));

    for (size_t i = 0; i < n; ++i) {
        logger->info(QString(stringArray[i]));
    }

    try {
        dev1.SetCamera(0);            // Open device
        dev1.SetPreserveRates(false); // Ask always for the best performances (FPS)
        dev1.SetColorCoding(CAlkUSB3::ColorCoding::Mono8);
        dev1.SetEnableImageThread(false);

        dev1.SetHorizontalBinning(2);
        dev1.SetVerticalBinning(2);
    } catch (CAlkUSB3::Exception e) {
        logger->warning(e.Message());
    }

    dev1.RawFrameAcquired().SetUserData(this);
    dev1.RawFrameAcquired().SetCallbackEx(&Autofocus::onFrameAcquired);

    ICeleraCamera& dev2 = CAlkUSB3::ICeleraCamera::Create();

    try {
        dev2.SetCamera(1);            // Open device
        dev2.SetPreserveRates(false); // Ask always for the best performances (FPS)
        dev2.SetColorCoding(CAlkUSB3::ColorCoding::Mono8);
        dev2.SetEnableImageThread(false);

        dev2.SetHorizontalBinning(2);
        dev2.SetVerticalBinning(2);
    } catch (CAlkUSB3::Exception e) {
        logger->warning(e.Message());
    }

    dev2.RawFrameAcquired().SetUserData(this);
    dev2.RawFrameAcquired().SetCallbackEx(&Autofocus::onFrameAcquired);
}

void Autofocus::start()
{
    if (!enabled) {
        return;
    }
    if (dev1.GetAcquire() && dev2.GetAcquire() ) {
        stop();
    }
    dev1.SetEnableImageThread(false); // No image conversion needed
    dev1.SetFrameRate(frameRate);
    dev1.SetShutter(exposureTime_us);

    dev2.SetEnableImageThread(false); 
    dev2.SetFrameRate(frameRate);
    dev2.SetShutter(exposureTime_us);

    try {
        dev1.SetAcquire(true);
        dev2.SetAcquire(true);
    } catch (CAlkUSB3::InvalidOperationException e) {
        throw std::runtime_error("Alkeria: invalid operation");
    }
}

void Autofocus::stop()
{
    dev1.SetAcquire(false);
    dev2.SetAcquire(false);
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

    CAlkUSB3::IVideoSource &videoSource(af->dev);
    CAlkUSB3::IVideoSource &videoSource2(af->dev2);
    CAlkUSB3::BufferPtr ptr1 = videoSource.GetRawDataPtr(false);
    CAlkUSB3::BufferPtr ptr2 = videoSource2.GetRawDataPtr(false);

    QList<CAlkUSB3::BufferPtr> ptr = {ptr1,ptr2};

    emit af->newImage(ptr);
    
    try {
        deltaList = af->getDelta();
    } catch (std::runtime_error e) {
        emit af->newStatus(e.what());
        return;
    }
    double alpha = af->mAlpha *mean().deltaList() + af->qAlpha;
    double beta1 = af->mbeta1 *(deltaList[1]-deltaList[0] + deltaList[3]-deltaList[2])/2 + af->qbeta1;
    double beta2 = af->mbeta2 *(deltaList[0]-deltaList[2] + deltaList[1]-deltaList[3])/2 + af->qbeta2;
    QList<double> correctionList = {alpha, beta1, beta2};
        
    if (af->isOutputEnabled()) {
        emit af->newCorrection(correctionList);
    }
    emit af->newStatus(QString("dx = %1, corr = %2").arg(deltaList).arg(correctionList));
}

QList<double> Autofocus::getDelta()
{
    CAlkUSB3::IVideoSource &videoSource(dev1);
    CAlkUSB3::IVideoSource &videoSource2(dev2);
    CAlkUSB3::BufferPtr ptr1 = videoSource1.GetRawDataPtr(false);
    CAlkUSB3::BufferPtr ptr2 = videoSource2.GetRawDataPtr(false);

    if (!ptr1 || !ptr2) {
        throw std::runtime_error("No frame was received");
    }

    cd_height = ptr1.GetHeight()  // ptr1 and ptr2 will have same dimensions so...
    cd_width = ptr1.GetWidth()
        
    Mat img1(cd_height, cd_width, CV_8U, (void *) ptr1.Data());
    Mat img2(cd_height, cd_width, CV_8U, (void *) ptr2.Data()); 

    int x1 = static_cast<int>(cd_width*0.05)          // 5% of img_width
    int x2 = static_cast<int>(cd_width*0.6)   // 60% of img_width
    int y1 = static_cast<int>(cd_height*0.6)  // 60% of img_height
    int y2 = static_cast<int>(cd_height*0.05)         // 5% of img_height
    
    int roi_width = static_cast<int>(cd_width*0.4)
    int roi_height = static_cast<int>(cd_height*0.4)

    cv::Rect roi1(x1, y1, roi_width, roi_height); // upperLeft ROI
    cv::Rect roi2(x2, y1, roi_width, roi_height); // upperRight ROI
    cv::Rect roi3(x1, y2, roi_width, roi_height); // buttomLeft ROI
    cv::Rect roi4(x2, y2, roi_width, roi_height); // buttomRightROI

    QList<cv::Mat> roi = {roi1, roi2, roi3, roi4}

    Mat couple1[2] = {img(roi[0]),img2(roi[0])};  // delta1
    Mat couple2[2] = {img(roi[1]),img2(roi[1])};  // delta2
    Mat couple3[2] = {img(roi[2]),img2(roi[2])};  // delta3
    Mat couple4[2] = {img(roi[3]),img2(roi[3])};  // delta4

    if (imageQualityEnabled) {
        if (!rapid_af::checkImageQuality(couple1[0], iqOptions)
            || !rapid_af::checkImageQuality(couple1[1], iqOptions)
            || !rapid_af::checkImageQuality(couple2[0], iqOptions)
            || !rapid_af::checkImageQuality(couple2[1], iqOptions)
            || !rapid_af::checkImageQuality(couple3[0], iqOptions)
            || !rapid_af::checkImageQuality(couple3[1], iqOptions)
            || !rapid_af::checkImageQuality(couple4[0], iqOptions)
            || !rapid_af::checkImageQuality(couple4[1], iqOptions)) {
            throw std::runtime_error("low image quality");
        }
    }

    bool ok = false;
    Point2f shift1;
    Point2f shift2;
    Point2f shift3;
    Point2f shift4;
    try {
        shift1 = rapid_af::align(couple1[0], couple1[1], options, &ok);
        shift2 = rapid_af::align(couple2[0], couple2[1], options, &ok);
        shift3 = rapid_af::align(couple3[0], couple3[1], options, &ok);
        shift4 = rapid_af::align(couple4[0], couple4[1], options, &ok);
    } catch (cv::Exception e) {
        logger->warning(e.what());
        throw std::runtime_error("OpenCV exception (see log)");
    }
    if (!ok) {
        throw std::runtime_error("Agreement threshold not met");
    }
    
    QList<double> shifts;
    shifts << shift1.x << shift2.x << shifts3.x << shifts4.x;
    return shifts;
}

double Autofocus::inferCalibrationQAlpha()
{    
    qAlpha = -mAlpha* correction[0];
    return qAlpha;
}

double Autofocus::inferCalibrationQBeta1()
{    
    qBeta1 = -mBeta1* correction[1];
    return qBeta1;
}

double Autofocus::inferCalibrationQBeta2()
{    
    qBeta2 = -mBeta2* correction[2];
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


QList<QImage> Autofocus::getMergedImage()
{
    QList<Mat> mergedImages;
    Mat img1
    int width = qMin(roi[0].width(), roi[0].width()) - 1;
    int height = qMin(roi[0].height(), roi[0].height()) - 1;   // All ROIs have same dimensions
    try {
        img1 = rapid_af::merge(couple1[0](Range(0, height), Range(0, width)),
                              couple1[1](Range(0, height), Range(0, width)),
                              deltaList[0]);
    } catch (cv::Exception e) {
    };
        
    Mat rgb;
    cv::cvtColor(img1, rgb, cv::COLOR_BGR2RGB);
    QImage qimg1 = QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
    mergedImages.append(qimg1)
        
    Mat img2
    try {
        img2 = rapid_af::merge(couple2[0](Range(0, height), Range(0, width)),
                              couple2[1](Range(0, height), Range(0, width)),
                              deltaList[1]);
    } catch (cv::Exception e) {
    };

    cv::cvtColor(img2, rgb, cv::COLOR_BGR2RGB);
    QImage qimg2 = QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
    mergedImages.append(qimg2)

    Mat img3
    try {
        img3 = rapid_af::merge(couple3[0](Range(0, height), Range(0, width)),
                              couple3[1](Range(0, height), Range(0, width)),
                              deltaList[2]);
    } catch (cv::Exception e) {
    };

    cv::cvtColor(img3, rgb, cv::COLOR_BGR2RGB);
    QImage qimg3 = QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
    mergedImages.append(qimg3)

    Mat img4
    try {
        img4 = rapid_af::merge(couple4[0](Range(0, height), Range(0, width)),
                              couple4[1](Range(0, height), Range(0, width)),
                              deltaList[3]);
    } catch (cv::Exception e) {
    };

    cv::cvtColor(img4, rgb, cv::COLOR_BGR2RGB);
    QImage qimg4 = QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
    mergedImages.append(qimg4)

    return mergedImages;
}
