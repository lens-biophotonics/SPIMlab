#ifndef AUTOFOCUS_H
#define AUTOFOCUS_H

#include <ICeleraCamera.h>

#include <QObject>

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

    double getCalibration_mAlpha() const;
    double getCalibration_qAlpha() const;
    double getCalibration_mBeta1() const;
    double getCalibration_qBeta1() const;
    double getCalibration_mBeta2() const;
    double getCalibration_qBeta2() const;

    void setCalibrationAlpha(double mAlpha, double qAlpha);
    void setCalibrationBeta1(double mBeta1, double qBeta1);
    void setCalibrationBeta2(double mBeta2, double qBeta2);

    QList<double> inferCalibrationQAlpha();
    QList<double> inferCalibrationQBeta1();
    QList<double> inferCalibrationQBeta2();

    bool isEnabled() const;
    void setEnabled(bool enable);

    bool isOutputEnabled() const;
    void setOutputEnabled(bool enable);

    QList<QImage> getMergedImage();

    rapid_af::AlignOptions getOptions() const;
    void setOptions(const rapid_af::AlignOptions &value);

    rapid_af::ImageQualityOptions getIqOptions() const;
    void setIqOptions(const rapid_af::ImageQualityOptions &value);

    bool isImageQualityEnabled() const;
    void setImageQualityEnabled(bool enable);

    QList<double> correctionList;

    int cd_width;
    int cd_height;

    QList<cv::Mat> roi;

    cv::Mat getImage1() const;
    cv::Mat getImage2() const;

signals:
    void newImage(QList<CAlkUSB3::BufferPtr>);
    void newStatus(QString);
    void newCorrection(QList<double>);

private:
    /**
     * @brief This object controls the camera.
     */
    CAlkUSB3::ICeleraCamera &dev1;
    CAlkUSB3::ICeleraCamera &dev2;
    rapid_af::AlignOptions options;
    rapid_af::ImageQualityOptions iqOptions;

    double exposureTime_us = 10000;
    double frameRate = 5;
    double mAlpha = 0;
    double qAlpha = 0;
    double mBeta1 = 0;
    double qBeta1 = 0;
    double mBeta2 = 0;
    double qBeta2 = 0;
    bool enabled = true;
    bool outputEnabled = true;
    bool imageQualityEnabled = true;

    static void onFrameAcquired(void *userData);

    QList<ICeleraCamera&> dev

    QList<double> getDelta();
    QList<cv::Point2f> shiftList;

    cv::Mat image1;
    cv::Mat image2;
};

#endif // AUTOFOCUS_H
