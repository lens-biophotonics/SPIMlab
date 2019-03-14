#ifndef SPIMHUB_H
#define SPIMHUB_H

#include <QObject>
#include <QThread>

#include "savestackworker.h"
#include "orcaflash.h"
#include "cameratrigger.h"
#include "galvoramp.h"

#include "pidevice.h"

#ifndef SPIM_NCAMS
#define SPIM_NCAMS 2
#endif

class SPIM : public QObject
{
    Q_OBJECT
public:
    enum PI_DEVICES : int {
        PI_DEVICE_X_AXIS,
        PI_DEVICE_Y_AXIS,
        PI_DEVICE_Z_AXIS,
        PI_DEVICE_LEFT_OBJ_AXIS,
        PI_DEVICE_RIGHT_OBJ_AXIS,
    };


    SPIM(QObject *parent = nullptr);
    virtual ~SPIM();

    OrcaFlash *getCamera(int camNumber) const;
    void setCamera(OrcaFlash *getCamera);

    void setupGalvoRampTriggerSource(const QStringList &terminals);

    PIDevice *piDevice(const PI_DEVICES dev) const;
    QList<PIDevice *> piDevices() const;

    CameraTrigger *getCameraTrigger() const;
    GalvoRamp *getGalvoRamp() const;

    double getExposureTime() const;
    void setExposureTime(double ms);

public slots:
    void startFreeRun();
    void startAcquisition();
    void stop();
    void initialize();
    void uninitialize();

signals:
    void initialized() const;
    void captureStarted() const;
    void stopped() const;
    void error(const QString) const;

private:
    QThread *acqThread = nullptr;
    SaveStackWorker *worker = nullptr;
    CameraTrigger *cameraTrigger = nullptr;
    GalvoRamp *galvoRamp = nullptr;

    double exposureTime;  // in ms

    QList<PIDevice *>piDevList;
    QList<OrcaFlash *>camList;

    void _setExposureTime(double expTime);

private slots:
    void onError(const QString &errMsg);
};

SPIM& spim();

#endif // SPIMHUB_H
