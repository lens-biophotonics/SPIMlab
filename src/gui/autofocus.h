#ifndef AUTOFOCUS_H
#define AUTOFOCUS_H

#include <ICeleraCamera.h>

#include <QObject>
#include <QRectF>

#include <rapid-af.h>

class Autofocus : public QObject
{
    Q_OBJECT
public:
    explicit Autofocus(QObject *parent = nullptr);

    void init();

    void start();
    void stop();

    double getExposureTime_us() const;
    void setExposureTime_us(double value);

    double getFrameRate() const;
    void setFrameRate(double value);

    double getCalibration_m() const;
    double getCalibration_q() const;

    void setCalibration(double m, double q);

    double inferCalibrationQ();

    bool isEnabled() const;
    void setEnabled(bool enable);

    bool isOutputEnabled() const;
    void setOutputEnabled(bool enable);

    QRect getLeftRoi() const;
    void setLeftRoi(const QRect &value);

    QRect getRightRoi() const;
    void setRightRoi(const QRect &value);

    cv::Mat getLeftRoiImage() const;
    cv::Mat getRightRoiImage() const;

    QImage getMergedImage();

    rapid_af::AlignOptions getOptions() const;
    void setOptions(const rapid_af::AlignOptions &value);

    rapid_af::ImageQualityOptions getIqOptions() const;
    void setIqOptions(const rapid_af::ImageQualityOptions &value);

    bool isImageQualityEnabled() const;
    void setImageQualityEnabled(bool enable);

signals:
    void newImage(CAlkUSB3::BufferPtr);
    void newStatus(QString);
    void newCorrection(double);

private:
    /**
     * @brief This object controls the camera.
     */
    CAlkUSB3::ICeleraCamera &dev;
    rapid_af::AlignOptions options;
    rapid_af::ImageQualityOptions iqOptions;

    double exposureTime_us = 10000;
    double frameRate = 5;
    double m = 0;
    double q = 0;
    bool enabled = true;
    bool outputEnabled = true;
    bool imageQualityEnabled = true;

    cv::Point2f shift;

    QRect leftRoi;
    QRect rightRoi;

    cv::Mat i1, i2;

    static void onFrameAcquired(void *userData);
    double getDelta();
};

#endif // AUTOFOCUS_H
