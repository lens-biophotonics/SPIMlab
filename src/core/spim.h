#ifndef SPIMHUB_H
#define SPIMHUB_H

#include <QObject>
#include <QThread>

#ifndef SPIM_NCAMS
#define SPIM_NCAMS 2
#endif
#ifndef SPIM_NCOBOLT
#define SPIM_NCOBOLT 4
#endif

class SaveStackWorker;
class OrcaFlash;
class CameraTrigger;
class GalvoRamp;
class PIDevice;
class Cobolt;

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

    PIDevice *getPIDevice(const PI_DEVICES dev) const;
    PIDevice *getPIDevice(const int dev) const;
    QList<PIDevice *> getPIDevices() const;

    CameraTrigger *getCameraTrigger() const;
    GalvoRamp *getGalvoRamp() const;

    double getExposureTime() const;
    void setExposureTime(double ms);

    QList<Cobolt *> getLaserDevices() const;
    Cobolt *getLaser(const int n) const;

public slots:
    void startFreeRun();
    void startAcquisition();
    void startCapture(bool freeRun = true);
    void stop();
    void initialize();
    void uninitialize();

signals:
    void initialized() const;
    void captureStarted() const;
    void stopped() const;
    void error(const QString) const;

private:
    QList<QThread *> acqThreads;
    CameraTrigger *cameraTrigger = nullptr;
    GalvoRamp *galvoRamp = nullptr;

    double exposureTime;  // in ms

    QList<PIDevice *>piDevList;
    QList<OrcaFlash *>camList;
    QList<Cobolt *>laserList;

    void _setExposureTime(double expTime);

private slots:
    void onError(const QString &errMsg);
};

SPIM& spim();

#endif // SPIMHUB_H
