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

    dev.SetCamera(0);            // Open device
    dev.SetPreserveRates(false); // Ask always for the best performances (FPS)
    dev.SetColorCoding(CAlkUSB3::ColorCoding::Mono8);
    dev.SetEnableImageThread(false);

    dev.SetHorizontalBinning(2);
    dev.SetVerticalBinning(2);

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

    CAlkUSB3::IVideoSource &videoSource(af->dev);
    CAlkUSB3::BufferPtr ptr = videoSource.GetRawDataPtr(false);

    emit af->newImage(ptr);

    double delta;
    try {
        delta = af->getDelta();
    } catch (std::runtime_error e) {
        emit af->newStatus(e.what());
        return;
    }

    double correction = af->m * delta + af->q;
    if (af->isOutputEnabled()) {
        emit af->newCorrection(correction);
    }
    emit af->newStatus(QString("dx = %1, corr = %2").arg(delta).arg(correction));
}

double Autofocus::getDelta()
{
    CAlkUSB3::IVideoSource &videoSource(dev);
    CAlkUSB3::BufferPtr ptr = videoSource.GetRawDataPtr(false);

    if (!ptr) {
        throw std::runtime_error("No frame was received");
    }

    Mat img(ptr.GetHeight(), ptr.GetWidth(), CV_8U, (void *) ptr.Data());
    i1 = img(Range(leftRoi.top(), leftRoi.bottom()), Range(leftRoi.left(), leftRoi.right()));
    i2 = img(Range(rightRoi.top(), rightRoi.bottom()), Range(rightRoi.left(), rightRoi.right()));

    if (imageQualityEnabled) {
        if (!rapid_af::checkImageQuality(i1, iqOptions)
            || !rapid_af::checkImageQuality(i2, iqOptions)) {
            throw std::runtime_error("low image quality");
        }
    }

    bool ok = false;
    Point2f delta;
    try {
        shift = rapid_af::align(i1, i2, options, &ok);
    } catch (cv::Exception e) {
        logger->warning(e.what());
        throw std::runtime_error("OpenCV exception (see log)");
    }
    if (!ok) {
        throw std::runtime_error("Agreement threshold not met");
    }

    return shift.x;
}

double Autofocus::inferCalibrationQ()
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
    QRect r = rightRoi;
    r.moveTopLeft(leftRoi.topLeft());
    r = r.intersected(leftRoi);
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
