#include "autofocus.h"

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
    , dev(CAlkUSB3::ICeleraCamera::Create())
{
    qRegisterMetaType<CAlkUSB3::BufferPtr>("CAlkUSB3::BufferPtr");
}

void Autofocus::init()
{
    auto stringArray = dev.GetCameraList();

    size_t n = stringArray.Size();

    if (n == 0) {
        throw std::runtime_error("Cannot find Alkeria camera.");
    }

    logger->info(QString("Found %1 Alkeria cameras").arg(n));

    for (size_t i = 0; i < n; ++i) {
        logger->info(QString(stringArray[i]));
    }

    try {
        dev.SetCamera(0);            // Open device
        dev.SetCamera(1);
        dev.SetPreserveRates(false); // Ask always for the best performances (FPS)
        dev.SetColorCoding(CAlkUSB3::ColorCoding::Mono8);
        dev.SetEnableImageThread(false);

        dev.SetHorizontalBinning(2);
        dev.SetVerticalBinning(2);
    } catch (CAlkUSB3::Exception e) {
        logger->warning(e.Message());
    }

    dev.RawFrameAcquired().SetUserData(this);
    dev.RawFrameAcquired().SetCallbackEx(&Autofocus::onFrameAcquired);
}

void Autofocus::start()
{
    if (!enabled) {
        return;
    }
    if (dev.GetAcquire()) {
        stop();
    }
    dev.SetEnableImageThread(false); // No image conversion needed
    dev.SetFrameRate(frameRate);
    dev.SetShutter(exposureTime_us);

    try {
        dev.SetAcquire(true);
    } catch (CAlkUSB3::InvalidOperationException e) {
        throw std::runtime_error("Alkeria: invalid operation");
    }
}

void Autofocus::stop()
{
    dev.SetAcquire(false);
}

/**
 * @brief This callback is called upon frame acquisition.
 * @param userData A pointer to this class object.
 */
void Autofocus::onFrameAcquired(void *userData)
{
    if (!userData)
        return;

    Autofocus *af = static_cast<Autofocus *>(userData);
    
    if (!af->upLeftRoi.isValid() || !af->upRightRoi.isValid() || !af->downLeftRoi.isValid() || !af->downRightRoi.isValid()) {
        emit af->newStatus("Invalid ROIs");
        return;
    }

    CAlkUSB3::IVideoSource &videoSource1 = dev.GetVideoSource(0);
    CAlkUSB3::IVideoSource &videoSource2 = dev.GetVideoSource(1);
    
    CAlkUSB3::BufferPtr ptr1 = videoSource1.GetRawDataPtr(false);
    CAlkUSB3::BufferPtr ptr2 = videoSource2.GetRawDataPtr(false);
    
    emit af->newImage1(ptr1);
    emit af->newImage2(ptr2);

    double delta1;
    try {
        delta1 = af->getDelta(1);
    } catch (std::runtime_error e) {
        emit af->newStatus(e.what());
        return;
    }
    double correction1 = af->m * delta1 + af->q;
    if (af->isOutputEnabled()) {
        emit af->newCorrection1(correction1);
    }
    emit af->newStatus(QString("dx = %1, corr = %2").arg(delta1).arg(correction1));

    double delta2;
    try {
        delta2 = af->getDelta(2);
    } catch (std::runtime_error e) {
        emit af->newStatus(e.what());
        return;
    }

    double correction2 = af->m * delta2 + af->q;
    if (af->isOutputEnabled()) {
        emit af->newCorrection2(correction2);
    }
    emit af->newStatus(QString("dx = %1, corr = %2").arg(delta2).arg(correction2));

    double delta3;
    try {
        delta3 = af->getDelta(3);
    } catch (std::runtime_error e) {
        emit af->newStatus(e.what());
        return;
    }

    double correction3 = af->m * delta3 + af->q;
    if (af->isOutputEnabled()) {
        emit af->newCorrection3(correction3);
    }
    emit af->newStatus(QString("dx = %1, corr = %2").arg(delta3).arg(correction3));

    double delta4;
    try {
        delta4 = af->getDelta(4);
    } catch (std::runtime_error e) {
        emit af->newStatus(e.what());
        return;
    }

    double correction4 = af->m * delta4 + af->q;
    if (af->isOutputEnabled()) {
        emit af->newCorrection4(correction4);
    }
    emit af->newStatus(QString("dx = %1, corr = %2").arg(delta4).arg(correction4));
}

double Autofocus::getDelta()
{
    
    CAlkUSB3::IVideoSource &videoSource1 = dev.GetVideoSource(0);
    CAlkUSB3::IVideoSource &videoSource2 = dev.GetVideoSource(1);
    
    CAlkUSB3::BufferPtr ptr1 = videoSource1.GetRawDataPtr(false);
    CAlkUSB3::BufferPtr ptr2 = videoSource2.GetRawDataPtr(false);

    if (!ptr1 || !ptr2) {
        throw std::runtime_error("No frame was received");
    }

    Mat img1(ptr1.GetHeight(), ptr1.GetWidth(), CV_8U, (void *) ptr1.Data());  // from ICelera1
    Mat img2(ptr2.GetHeight(), ptr2.GetWidth(), CV_8U, (void *) ptr2.Data()); // from ICelera2
    
    i1 = img1(Range(upLeftRoi.top(), upLeftRoi.bottom()), Range(upLeftRoi.left(), upLeftRoi.right()));
    i2 = img2(Range(upLeftRoi.top(), upLeftRoi.bottom()), Range(upLeftRoi.left(), upLeftRoi.right()));
    
    i3 = img1(Range(upRightRoi.top(), upRightRoi.bottom()), Range(upRightRoi.left(), upRightRoi.right()));
    i4 = img2(Range(upRightRoi.top(), upRightRoi.bottom()), Range(upRightRoi.left(), upRightRoi.right()));
    
    i5 = img1(Range(downLeftRoi.top(), downLeftRoi.bottom()), Range(downLeftRoi.left(), downLeftRoi.right()));
    i6 = img2(Range(downLeftRoi.top(), downLeftRoi.bottom()), Range(downLeftRoi.left(), downLeftRoi.right())); 
    
    i7 = img1(Range(downRightRoi.top(), downRightRoi.bottom()), Range(downRightRoi.left(), downRightRoi.right()));
    i8 = img2(Range(downRightRoi.top(), downRightRoi.bottom()), Range(downRightRoi.left(), downRightRoi.right()));
    
    if (imageQualityEnabled) {
        if (!rapid_af::checkImageQuality(i1, iqOptions)
            || !rapid_af::checkImageQuality(i2, iqOptions) 
            || !rapid_af::checkImageQuality(i3, iqOptions) 
            || !rapid_af::checkImageQuality(i4, iqOptions) 
            || !rapid_af::checkImageQuality(i5, iqOptions) 
            || !rapid_af::checkImageQuality(i6, iqOptions) 
            || !rapid_af::checkImageQuality(i7, iqOptions) 
            || !rapid_af::checkImageQuality(i8, iqOptions)) {
            throw std::runtime_error("low image quality");
        }
    }
    
    Point2f shift
    if (deltaNumber == 1) {
        bool ok1 = false;
        try {
            shift = rapid_af::align(i1, i2, options, &ok);
        } catch (cv::Exception e) {
            logger->warning(e.what());
            throw std::runtime_error("OpenCV exception (see log)");
        }
        if (!ok1) {
            throw std::runtime_error("Agreement threshold not met");
        }     
    }
    
    if (deltaNumber == 2) {
        bool ok2 = false;
        try {
            shift = rapid_af::align(i3, i4, options, &ok);
        } catch (cv::Exception e) {
            logger->warning(e.what());
            throw std::runtime_error("OpenCV exception (see log)");
        }
        if (!ok2) {
            throw std::runtime_error("Agreement threshold not met");
        } 
    }

    if (deltaNumber == 3) {
        bool ok3 = false;
        try {
            shift = rapid_af::align(i5, i6, options, &ok);
        } catch (cv::Exception e) {
            logger->warning(e.what());
            throw std::runtime_error("OpenCV exception (see log)");
        }
        if (!ok3) {
            throw std::runtime_error("Agreement threshold not met");
        }     
    }
    
    if (deltaNumber == 4) {
        bool ok4 = false;
        try {
            shift = rapid_af::align(i7, i8, options, &ok);
        } catch (cv::Exception e) {
            logger->warning(e.what());
            throw std::runtime_error("OpenCV exception (see log)");
        }
        if (!ok4) {
            throw std::runtime_error("Agreement threshold not met");
        } 
    }

    return shift.x;
}

double Autofocus::inferCalibrationQ(int deltaNumber)
{
    if (deltaNumber == 1){
        q = -m * getDelta(1);
    }
    if (deltaNumber == 2){
        q = -m * getDelta(2);
    }
    if (deltaNumber == 3){
        q = -m * getDelta(3);
    }
    if (deltaNumber == 4){
        q = -m * getDelta(4);
    }
    
    return q;
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

double Autofocus::getCalibration_m() const
{
    return m;
}

void Autofocus::setCalibration(double m, double q)
{
    this->m = m;
    this->q = q;
}

double Autofocus::getCalibration_q() const
{
    return q;
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

QRect Autofocus::getUpLeftRoi() const
{
    return upLeftRoi;
}

void Autofocus::setUpLeftRoi(const QRect &value)
{
    upLeftRoi = value;
}

QRect Autofocus::getUpRightRoi() const
{
    return upRightRoi;
}

void Autofocus::setUpRightRoi(const QRect &value)
{
    upRightRoi = value;
}

QRect Autofocus::getDownLeftRoi() const
{
    return downLeftRoi;
}

void Autofocus::setDownLeftRoi(const QRect &value)
{
    downLeftRoi = value;
}

QRect Autofocus::getDownRightRoi() const
{
    return downRightRoi;
}

void Autofocus::setDownRightRoi(const QRect &value)
{
    downRightRoi = value;
}

cv::Mat Autofocus::getUpLeftRoiImagecam1() const
{
    return i1;
}

cv::Mat Autofocus::getUpLeftRoiImagecam2() const
{
    return i2;
}

cv::Mat Autofocus::getUpRightRoiImagecam1() const
{
    return i3;
}

cv::Mat Autofocus::getUpRightRoiImagecam2() const
{
    return i4;
}

cv::Mat Autofocus::getDownLeftRoiImagecam1() const
{
    return i5;
}

cv::Mat Autofocus::getDownLeftRoiImagecam2() const
{
    return i6;
}

cv::Mat Autofocus::getDownRightRoiImagecam1() const
{
    return i7;
}

cv::Mat Autofocus::getDownRightRoiImagecam2() const
{
    return i8;
}


QImage Autofocus::getMergedImage(int deltaNumber)
{
    Mat img;
    int width = qMin(leftRoi.width(), rightRoi.width()) - 1;
    int height = qMin(leftRoi.height(), rightRoi.height()) - 1;
    try {
        img = rapid_af::merge(i1(Range(0, height), Range(0, width)),
                              i2(Range(0, height), Range(0, width)),
                              shift);
    } catch (cv::Exception e) {
    };

    Mat rgb;
    cv::cvtColor(img, rgb, cv::COLOR_BGR2RGB);
    QImage qimg = QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();

    return qimg;
}
