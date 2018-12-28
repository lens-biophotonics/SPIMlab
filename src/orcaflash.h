#ifndef ORCAFLASH_H
#define ORCAFLASH_H

#include <QObject>

namespace DCAM {
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
        TRIGMODE_INTERNAL = DCAM::DCAM_TRIGMODE_INTERNAL,
        TRIGMODE_EDGE = DCAM::DCAM_TRIGMODE_EDGE,
        TRIGMODE_LEVEL = DCAM::DCAM_TRIGMODE_LEVEL,
        TRIGMODE_SOFTWARE = DCAM::DCAM_TRIGMODE_SOFTWARE,
        TRIGMODE_TDI = DCAM::DCAM_TRIGMODE_TDI,
        TRIGMODE_TDIINTERNAL = DCAM::DCAM_TRIGMODE_TDIINTERNAL,
        TRIGMODE_START = DCAM::DCAM_TRIGMODE_START,
        TRIGMODE_SYNCREADOUT = DCAM::DCAM_TRIGMODE_SYNCREADOUT,
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

    explicit OrcaFlash(QObject *parent = nullptr);
    virtual ~OrcaFlash();
    bool open(int index);
    bool close();

    bool startCapture(int32_t framecount = 10);
    bool stop();

    bool copyFrame(void *buf, size_t n, int32_t frame);
    bool copyLastFrame(void *buf, size_t n);

    double getExposureTime();
    bool setExposureTime(double sec);

    ORCA_TRIGGER_MODE getTriggerMode();
    bool setTriggerMode(ORCA_TRIGGER_MODE mode);

    ORCA_TRIGGER_POLARITY getTriggerPolarity();
    bool setTriggerPolarity(ORCA_TRIGGER_POLARITY polarity);

    ORCA_STATUS getStatus();

    QString getLastError();

    bool setGet(DCAM::_DCAMIDPROP property, double value, double *get);

signals:

public slots:

private:
#ifdef DCAMAPI_HEADERS
    DCAM::HDCAM h;
#endif
    int nCamera;
    double exposureTime;
    ORCA_TRIGGER_MODE triggerMode;
    ORCA_TRIGGER_POLARITY triggerPolarity;

    void logLastError(QString label = "");
};

#endif // ORCAFLASH_H
