#ifndef ORCAFLASH_H
#define ORCAFLASH_H

#include <QObject>
#include <QMutex>

#include "dcamutils.h"


class OrcaFlash : public QObject
{
    Q_OBJECT
public:
    enum ORCA_TRIGGER_MODE {
        TRIGMODE_INTERNAL = DCAM::DCAM_TRIGMODE_INTERNAL,
        TRIGMODE_EDGE = DCAM::DCAM_TRIGMODE_EDGE,
        TRIGMODE_LEVEL = DCAM::DCAM_TRIGMODE_LEVEL,
        TRIGMODE_SOFTWARE = DCAM::DCAM_TRIGMODE_SOFTWARE,
        TRIGMODE_TDI = DCAM::DCAM_TRIGMODE_TDI,
        TRIGMODE_TDIINTERNAL = DCAM::DCAM_TRIGMODE_TDIINTERNAL,
        TRIGMODE_START = DCAM::DCAM_TRIGMODE_START,
        TRIGMODE_SYNCREADOUT = DCAM::DCAM_TRIGMODE_SYNCREADOUT,
    };

    enum ORCA_OUTPUT_TRIGGER_SOURCE {
        OUTPUT_TRIGGER_SOURCE_EXPOSURE = DCAM::DCAMPROP_OUTPUTTRIGGER_SOURCE__EXPOSURE,
        OUTPUT_TRIGGER_SOURCE_READOUTEND = DCAM::DCAMPROP_OUTPUTTRIGGER_SOURCE__READOUTEND,
        OUTPUT_TRIGGER_SOURCE_VSYNC = DCAM::DCAMPROP_OUTPUTTRIGGER_SOURCE__VSYNC,
        OUTPUT_TRIGGER_SOURCE_HSYNC = DCAM::DCAMPROP_OUTPUTTRIGGER_SOURCE__HSYNC,
        OUTPUT_TRIGGER_SOURCE_TRIGGER = DCAM::DCAMPROP_OUTPUTTRIGGER_SOURCE__TRIGGER,
    };

    enum ORCA_OUTPUT_TRIGGER_KIND {
        OUTPUT_TRIGGER_KIND_LOW = DCAM::DCAMPROP_OUTPUTTRIGGER_KIND__LOW,
        OUTPUT_TRIGGER_KIND_EXPOSURE = DCAM::DCAMPROP_OUTPUTTRIGGER_KIND__EXPOSURE,
        OUTPUT_TRIGGER_KIND_PROGRAMMABLE = DCAM::DCAMPROP_OUTPUTTRIGGER_KIND__PROGRAMABLE,
        OUTPUT_TRIGGER_KIND_TRIGGERREADY = DCAM::DCAMPROP_OUTPUTTRIGGER_KIND__TRIGGERREADY,
        OUTPUT_TRIGGER_KIND_HIGH = DCAM::DCAMPROP_OUTPUTTRIGGER_KIND__HIGH,
    };

    enum ORCA_TRIGGER_POLARITY {
        POL_NEGATIVE = DCAM::DCAM_TRIGPOL_NEGATIVE,
        POL_POSITIVE = DCAM::DCAM_TRIGPOL_POSITIVE,
    };

    enum ORCA_STATUS {
        STATUS_BUSY = DCAM::DCAM_STATUS_BUSY,
        STATUS_ERROR = DCAM::DCAM_STATUS_ERROR,
        STATUS_READY = DCAM::DCAM_STATUS_READY,
        STATUS_STABLE = DCAM::DCAM_STATUS_STABLE,
        STATUS_UNSTABLE = DCAM::DCAM_STATUS_UNSTABLE,
    };

    enum ORCA_SENSOR_MODE {
        SENSOR_MODE_AREA = DCAM::DCAMPROP_SENSORMODE__AREA,
        SENSOR_MODE_SLIT= DCAM::DCAMPROP_SENSORMODE__SLIT,
        SENSOR_MODE_LINE= DCAM::DCAMPROP_SENSORMODE__LINE,
        SENSOR_MODE_TDI= DCAM::DCAMPROP_SENSORMODE__TDI,
        SENSOR_MODE_FRAMING= DCAM::DCAMPROP_SENSORMODE__FRAMING,
        SENSOR_MODE_PARTIAL_AREA = DCAM::DCAMPROP_SENSORMODE__PARTIALAREA,
        SENSOR_MODE_SLIT_LINE = DCAM::DCAMPROP_SENSORMODE__SLITLINE,
        SENSOR_MODE_TDI_EXTENDED = DCAM::DCAMPROP_SENSORMODE__TDI_EXTENDED,
        SENSOR_MODE_PANORAMIC = DCAM::DCAMPROP_SENSORMODE__PANORAMIC,
        SENSOR_MODE_PROGRESSIVE = DCAM::DCAMPROP_SENSORMODE__PROGRESSIVE,
        SENSOR_MODE_SPLIT_VIEW = DCAM::DCAMPROP_SENSORMODE__SPLITVIEW,
        SENSOR_MODE_DUAL_LIGHTSHEET = DCAM::DCAMPROP_SENSORMODE__DUALLIGHTSHEET,
    };

    explicit OrcaFlash(QObject *parent = nullptr);
    virtual ~OrcaFlash();
    void open(int index);
    void close();
    DCAM::ModelInfo *modelInfo();

    void setNFramesInBuffer(uint count);
    uint nFramesInBuffer();

    void startCapture();
    void stop();

    void copyFrame(void *buf, size_t n, int32_t frame);
    void copyLastFrame(void *buf, size_t n);
    void wait(DCAM::_DWORD timeout = 1000,
              DCAM::DCAMWAIT_EVENT event = DCAM::DCAMCAP_EVENT_FRAMEREADY);
    void lockData(void **pTop, int32_t *pRowbytes, int32_t frame);
    void unlockData();

    double getExposureTime();
    double setGetExposureTime(double sec);

    ORCA_TRIGGER_MODE getTriggerMode();
    ORCA_TRIGGER_MODE setGetTriggerMode(ORCA_TRIGGER_MODE mode);

    ORCA_TRIGGER_POLARITY getTriggerPolarity();
    ORCA_TRIGGER_POLARITY setGetTriggerPolarity(ORCA_TRIGGER_POLARITY polarity);

    ORCA_STATUS getStatus();
    QString getStatusString();
    static QString statusString(OrcaFlash::ORCA_STATUS status);
    void logStatusString();

    QString getLastError();

    double setGet(DCAM::_DCAMIDPROP property, double value);
    double getPropertyValue(DCAM::_DCAMIDPROP property);
    void setPropertyValue(DCAM::_DCAMIDPROP property, double value);
    double getLineInterval();
    void setOutputTrigger(ORCA_OUTPUT_TRIGGER_KIND kind,
                          ORCA_OUTPUT_TRIGGER_SOURCE source);
    void setSensorMode(ORCA_SENSOR_MODE mode);
    int nOfLines();

signals:

public slots:

private:
#ifdef DCAMAPI_HEADERS
    DCAM::HDCAM h;
#endif
    int cameraIndex;
    double exposureTime;
    ORCA_TRIGGER_MODE triggerMode;
    ORCA_TRIGGER_POLARITY triggerPolarity;

    QMutex mutex;
    uint _nFramesInBuffer;
    bool _isOpen;

    QString logLastError(QString label = "");
};

#endif // ORCAFLASH_H
