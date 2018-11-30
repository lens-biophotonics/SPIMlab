#ifndef ORCAFLASH_H
#define ORCAFLASH_H

#include <QObject>

namespace HAMAMATSU {
#ifdef DCAMAPI_HEADERS
#include <dcamapi.h>
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

    QString getLastError();

signals:

public slots:

private:
#ifdef DCAMAPI_HEADERS
    HAMAMATSU::HDCAM h;
#endif
    int nCamera;
    double exposureTime;
    ORCA_TRIGGER_MODE triggerMode;

    void logLastError(QString label = "");
};

#endif // ORCAFLASH_H
