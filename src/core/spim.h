#ifndef SPIMHUB_H
#define SPIMHUB_H

#include <QObject>
#include <QThread>
#include <QStateMachine>

#ifndef SPIM_NCAMS
#define SPIM_NCAMS 2
#endif
#ifndef SPIM_NCOBOLT
#define SPIM_NCOBOLT 4
#endif
#ifndef SPIM_NPIDEVICES
#define SPIM_NPIDEVICES 5
#endif

#define SPIM_RANGE_FROM_IDX 0
#define SPIM_RANGE_TO_IDX 1
#define SPIM_RANGE_STEP_IDX 2

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
    enum MACHINE_STATE {
        STATE_UNINITIALIZED,
        STATE_READY,
        STATE_CAPTURING,
        STATE_ERROR,

        STATE_ACQUISITION,
        STATE_CAPTURE,
        STATE_PRECAPTURE,

        STATE_FREERUN,
    };

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
    QList<OrcaFlash *> getCameras();
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

    PI_DEVICES getStackStage() const;

    QList<PI_DEVICES> getMosaicStages() const;

    QList<double> *getScanRange(const PI_DEVICES dev) const;

    QString getOutputPath() const;
    void setOutputPath(const QString &value);

    QState *getState(const MACHINE_STATE stateEnum);

    int getCurrentStep() const;
    int getTotalSteps() const;
    double getTriggerRate() const;

    int getNSteps(const PI_DEVICES devEnum) const;

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
    CameraTrigger *cameraTrigger = nullptr;
    GalvoRamp *galvoRamp = nullptr;

    double exposureTime;  // in ms
    double triggerRate;

    QList<PIDevice *>piDevList;
    QList<OrcaFlash *>camList;
    QList<Cobolt *>laserList;

    QStateMachine *sm = nullptr;

    PI_DEVICES stackStage;
    QList<PI_DEVICES> mosaicStages;

    int currentStep = 0;
    int totalSteps = 0;
    QMap<PI_DEVICES, int> nSteps;
    QMap<PI_DEVICES, QList<double>*> scanRangeMap;

    QString outputPath;

    bool freeRun = true;

    QMap<MACHINE_STATE, QState *> stateMap;

    void _setExposureTime(double expTime);
    void _startAcquisition();
    void setupStateMachine();

private slots:
    void onError(const QString &errMsg);
};

SPIM& spim();

#endif // SPIMHUB_H
