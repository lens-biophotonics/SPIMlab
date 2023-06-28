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
    ICeleraCamera& dev = CAlkUSB3::ICeleraCamera::Create() ;

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
    if (dev.GetAcquire() && dev2.GetAcquire() ) {
        stop();
    }
    dev.SetEnableImageThread(false); // No image conversion needed
    dev.SetFrameRate(frameRate);
    dev.SetShutter(exposureTime_us);

    dev2.SetEnableImageThread(false); // No image conversion needed
    dev2.SetFrameRate(frameRate);
    dev2.SetShutter(exposureTime_us);

    try {
        dev.SetAcquire(true);
        dev2.SetAcquire(true);
    } catch (CAlkUSB3::InvalidOperationException e) {
        throw std::runtime_error("Alkeria: invalid operation");
    }
}

void Autofocus::stop()
{
    dev.SetAcquire(false);
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
    
    if (!af->leftRoi.isValid() || !af->rightRoi.isValid()) {
        emit af->newStatus("Invalid ROIs");
        return;
    }

    CAlkUSB3::IVideoSource &videoSource(af->dev);
    CAlkUSB3::IVideoSource &videoSource2(af->dev2);
    CAlkUSB3::BufferPtr ptr = videoSource.GetRawDataPtr(false);
    CAlkUSB3::BufferPtr ptr2 = videoSource2.GetRawDataPtr(false);

    emit af->newImage(ptr);
    emit af->newImage(ptr2);
    
    QList<double> deltaList;
    try {
        deltaList = af->getDelta();
    } catch (std::runtime_error e) {
        emit af->newStatus(e.what());
        return;
    }

    QList<double> correctionList
    correctionList.reserve(deltaList.size());
    for (i=0; i<deltaList.size(); i++){
      correctionList.append(af->m * deltaList[i]+af->q[i])
    }
        
    if (af->isOutputEnabled()) {
        emit af->newCorrection(correctionList);
    }
    emit af->newStatus(QString("dx = %1, corr = %2").arg(deltaList).arg(correctionList));
}

QList<double> Autofocus::getDelta()
{
    CAlkUSB3::IVideoSource &videoSource(dev);
    CAlkUSB3::IVideoSource &videoSource2(dev2);
    CAlkUSB3::BufferPtr ptr = videoSource.GetRawDataPtr(false);
    CAlkUSB3::BufferPtr ptr2 = videoSource2.GetRawDataPtr(false);

    if (!ptr || !ptr2) {
        throw std::runtime_error("No frame was received");
    }

    Mat img(ptr.GetHeight(), ptr.GetWidth(), CV_8U, (void *) ptr.Data());
    Mat img2(ptr2.GetHeight(), ptr2.GetWidth(), CV_8U, (void *) ptr2.Data()); 

    Mat couple1[2] = {img(Range(upLeftRoi1.top(), upLeftRoi1.bottom()), Range(upLeftRoi1.left(), upLeftRoi1.right())),
                        img2(Range(upLeftRoi2.top(), upLeftRoi2.bottom()), Range(upLeftRoi2.left(), upLeftRoi2.right()))};
    Mat couple2[2] = {img(Range(upRightRoi1.top(), upRightRoi1.bottom()), Range(upRightRoi1.left(), upRightRoi1.right())),
                        img2(Range(upRightRoi2.top(), upRightRoi2.bottom()), Range(upRightRoi2.left(), upRightRoi2.right()))};
    Mat couple3[2] = {img(Range(downLeftRoi1.top(), downLeftRoi1.bottom()), Range(downLeftRoi1.left(), downLeftRoi1.right())),
                        img2(Range(downLeftRoi2.top(), downLeftRoi2.bottom()), Range(downLeftRoi2.left(), downLeftRoi2.right()))};
    Mat couple4[2] = {img(Range(downRightRoi1.top(), downRightRoi1.bottom()), Range(downRightRoi1.left(), downRightRoi1.right())),
                        img2(Range(downRightRoi2.top(), downRightRoi2.bottom()), Range(downRightRoi2.left(), downRightRoi2.right()))};     

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
    delta << shift1.x << shift2.x << shifts3.x << shifts4.x;
    return shifts;
}

QList<double> Autofocus::inferCalibrationQ()
{
    q = -m * getDelta();
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

void Autofocus::setCalibration(double m, QList<double> q)
{
    this->m = m;
    this->q = q;
}

QList<double> Autofocus::getCalibration_q() const
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

QRect Autofocus::getLeftRoi() const
{
    return leftRoi;
}

void Autofocus::setLeftRoi(const QRect &value)
{
    leftRoi = value;
}

QRect Autofocus::getRightRoi() const
{
    return rightRoi;
}

void Autofocus::setRightRoi(const QRect &value)
{
    rightRoi = value;
}

cv::Mat Autofocus::getLeftRoiImage() const
{
    return i1;
}

cv::Mat Autofocus::getRightRoiImage() const
{
    return i2;
}

QImage Autofocus::getMergedImage()
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
