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

    double inferCalibrationQ(int deltaNumber);

    bool isEnabled() const;
    void setEnabled(bool enable);

    bool isOutputEnabled() const;
    void setOutputEnabled(bool enable);

    QRect getUpLeftRoi() const;
    void setUpLeftRoi(const QRect &value);

    QRect getUpRightRoi() const;
    void setUpRightRoi(const QRect &value);

    QRect getDownLeftRoi() const;
    void setDownLeftRoi(const QRect &value);

    QRect getDownRightRoi() const;
    void setDownRightRoi(const QRect &value);

    cv::Mat getUpLeftRoiImagecam1() const;
    cv::Mat getUpLeftRoiImagecam2() const;
    cv::Mat getUpRightRoiImagecam1() const;
    cv::Mat getUpRightRoiImagecam2() const;
    cv::Mat getDownLeftRoiImagecam1() const;
    cv::Mat getDownLeftRoiImagecam2() const;
    cv::Mat getDownRightRoiImagecam1() const;
    cv::Mat getDownRightRoiImagecam2() const;

    QImage getMergedImage();

    rapid_af::AlignOptions getOptions() const;
    void setOptions(const rapid_af::AlignOptions &value);

    rapid_af::ImageQualityOptions getIqOptions() const;
    void setIqOptions(const rapid_af::ImageQualityOptions &value);

    bool isImageQualityEnabled() const;
    void setImageQualityEnabled(bool enable);

signals:
    void newImage1(CAlkUSB3::BufferPtr);
    void newImage2(CAlkUSB3::BufferPtr);
    void newStatus(QString);
    void newCorrection1(double);
    void newCorrection2(double);
    void newCorrection3(double);
    void newCorrection4(double);

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

    QRect upLeftRoi;
    QRect upRightRoi;
    QRect downLeftRoi;
    QRect downRightRoi;

    cv::Mat i1, i2, i3, i4, i5, i6, i7, i8;

    int deltaNumber

    static void onFrameAcquired(void *userData);
    double getDelta(int deltaNumber);
};

#endif // AUTOFOCUS_H
