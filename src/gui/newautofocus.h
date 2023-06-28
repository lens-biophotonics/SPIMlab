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
    QList<double> getCalibration_q() const;

    void setCalibration(double m, QList<double> q);

    QList<double> inferCalibrationQ();

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
    void newImage1(CAlkUSB3::BufferPtr);
    void newImage2(CAlkUSB3::BufferPtr);
    void newStatus(QString);
    void newCorrection(QList<double>);

private:
    /**
     * @brief This object controls the camera.
     */
    CAlkUSB3::ICeleraCamera &dev;
    CAlkUSB3::ICeleraCamera &dev2;
    rapid_af::AlignOptions options;
    rapid_af::ImageQualityOptions iqOptions;

    double exposureTime_us = 10000;
    double frameRate = 5;
    double m = 0;
    double q = 0;
    bool enabled = true;
    bool outputEnabled = true;
    bool imageQualityEnabled = true;

    QList<cv::Point2f> shifts;
    QList<cv::Point2f> shift1;
    QList<cv::Point2f> shift2;
    QList<cv::Point2f> shift3;
    QList<cv::Point2f> shift4;

    QRect leftRoi;
    QRect rightRoi;

    static void onFrameAcquired(void *userData);
    QList<double> getDelta();
};

#endif // AUTOFOCUS_H
