#ifndef ORCAFLASH_H
#define ORCAFLASH_H

#include <QObject>

namespace HAMAMATSU {
#ifdef DCAMAPI_HEADERS
#include <dcamapi.h>
#include <dcamprop.h>
#endif
}

int init_dcam();
bool uninit_dcam();

class OrcaFlash : public QObject
{
    Q_OBJECT
public:
    enum ORCA_TRIGGER_MODE {
        TRIGMODE_INTERNAL = HAMAMATSU::DCAM_TRIGMODE_INTERNAL,
        TRIGMODE_EDGE = HAMAMATSU::DCAM_TRIGMODE_EDGE,
        TRIGMODE_LEVEL = HAMAMATSU::DCAM_TRIGMODE_LEVEL,
        TRIGMODE_SOFTWARE = HAMAMATSU::DCAM_TRIGMODE_SOFTWARE,
        TRIGMODE_TDI = HAMAMATSU::DCAM_TRIGMODE_TDI,
        TRIGMODE_TDIINTERNAL = HAMAMATSU::DCAM_TRIGMODE_TDIINTERNAL,
        TRIGMODE_START = HAMAMATSU::DCAM_TRIGMODE_START,
        TRIGMODE_SYNCREADOUT = HAMAMATSU::DCAM_TRIGMODE_SYNCREADOUT,
    };

    enum ORCA_TRIGGER_POLARITY {
        POL_NEGATIVE = HAMAMATSU::DCAM_TRIGPOL_NEGATIVE,
        POL_POSITIVE = HAMAMATSU::DCAM_TRIGPOL_POSITIVE,
    };

    enum ORCA_STATUS {
        STATUS_BUSY = HAMAMATSU::DCAM_STATUS_BUSY,
        STATUS_ERROR = HAMAMATSU::DCAM_STATUS_ERROR,
        STATUS_READY = HAMAMATSU::DCAM_STATUS_READY,
        STATUS_STABLE = HAMAMATSU::DCAM_STATUS_STABLE,
        STATUS_UNSTABLE = HAMAMATSU::DCAM_STATUS_UNSTABLE,
    };

    explicit OrcaFlash(QObject *parent = nullptr);
    virtual ~OrcaFlash();
    bool open(int index);
    bool close();

    bool startFreeRun();
    bool stop();

    bool copyLastFrame(void *buf, size_t n);

    double getExposureTime();
    bool setExposureTime(double sec);

    ORCA_TRIGGER_MODE getTriggerMode();
    bool setTriggerMode(ORCA_TRIGGER_MODE mode);

    ORCA_TRIGGER_POLARITY getTriggerPolarity();
    bool setTriggerPolarity(ORCA_TRIGGER_POLARITY polarity);

    ORCA_STATUS getStatus();

    QString getLastError();

    bool setGet(HAMAMATSU::_DCAMIDPROP property, double value, double *get);

signals:

public slots:

private:
#ifdef DCAMAPI_HEADERS
    HAMAMATSU::HDCAM h;
#endif
    int nCamera;
    double exposureTime;
    ORCA_TRIGGER_MODE triggerMode;
    ORCA_TRIGGER_POLARITY triggerPolarity;

    void logLastError(QString label = "");
};

#endif // ORCAFLASH_H
