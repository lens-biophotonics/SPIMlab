#ifndef SPIMHUB_H
#define SPIMHUB_H

#include <QDir>
#include <QMap>
#include <QObject>
#include <QStateMachine>
#include <QThread>

#ifndef SPIM_NCAMS
#define SPIM_NCAMS 2
#endif
#ifndef SPIM_NCOBOLT
#define SPIM_NCOBOLT 4
#endif
#ifndef SPIM_NAOTF
#define SPIM_NAOTF 2
#endif
#ifndef SPIM_NPIDEVICES
#define SPIM_NPIDEVICES 5
#endif

#define SPIM_RANGE_FROM_IDX 0
#define SPIM_RANGE_TO_IDX 1
#define SPIM_RANGE_STEP_IDX 2

#define SPIM_SCAN_DECIMALS 5

class SaveStackWorker;
class OrcaFlash;

#ifdef MASTER_SPIM
class PIDevice;
class Cobolt;
class FilterWheel;
class AA_MPDSnCxx;
class Tasks;
#endif

enum SPIM_PI_DEVICES : int {
    PI_DEVICE_X_AXIS,
    PI_DEVICE_Y_AXIS,
    PI_DEVICE_Z_AXIS,
    PI_DEVICE_LEFT_OBJ_AXIS,
    PI_DEVICE_RIGHT_OBJ_AXIS,
};

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
        STATE_PRECAPTURE,
        STATE_CAPTURE,

        STATE_FREERUN,
    };

    SPIM(QObject *parent = nullptr);
    virtual ~SPIM();

    OrcaFlash *getCamera(int camNumber) const;
    QList<OrcaFlash *> getCameraDevices();
    void setCamera(OrcaFlash *getCamera);
    SaveStackWorker *getSSWorker(int camNumber);

    double getExposureTime() const;
    bool setExposureTime(double ms);

    SPIM_PI_DEVICES getStackStage() const;

    QList<SPIM_PI_DEVICES> getMosaicStages() const;

    QList<double> *getScanRange(const SPIM_PI_DEVICES dev) const;

    QStringList getOutputPathList() const;
    void setOutputPathList(const QStringList &sl);

    QState *getState(const MACHINE_STATE stateEnum);

    int getCurrentStep() const;
    int getTotalSteps() const;
    double getTriggerRate() const;

    int getNSteps(const SPIM_PI_DEVICES devEnum) const;

    double getScanVelocity() const;
    void setScanVelocity(double value);

    QString getRunName() const;
    bool setRunName(const QString &value);

    QDir getFullOutputDir(int cam);

    bool isMosaicStageEnabled(SPIM_PI_DEVICES dev) const;
    void setMosaicStageEnabled(SPIM_PI_DEVICES dev, bool enable);

    bool isCameraEnabled(uint dev);
    void setCameraEnabled(uint camera, bool enable);

    int getBinning() const;
    bool setBinning(uint value);

    bool getTurnOffLasersAtEndOfAcquisition() const;
    void setTurnOffLasersAtEndOfAcquisition(bool enable);

    bool setOutputFname(const QString &value);
    bool setFrameCount(int value);

    bool isSpimInitialized() const;

#ifdef MASTER_SPIM
    PIDevice *getPIDevice(const SPIM_PI_DEVICES dev) const;
    PIDevice *getPIDevice(const int dev) const;
    QList<PIDevice *> getPIDevices() const;

    QList<Cobolt *> getLaserDevices() const;
    Cobolt *getLaser(const int n) const;

    QList<FilterWheel *> getFilterWheelDevices() const;
    FilterWheel *getFilterWheel(const int n) const;

    AA_MPDSnCxx *getAOTF(int dev);

    Tasks *getTasks() const;

    void turnOffAllLasers();
    bool areLasersOn();
#endif

public slots:
    void startFreeRun();
    bool startAcquisition();
    void stop();
    void emergencyStop();
    bool initializeSpim();
    void uninitializeSpim();

#ifdef MASTER_SPIM
    void haltStages();
#endif

signals:
    void initialized() const;
    void captureStarted() const;
    void stopped() const;
    void error(const QString) const;
    void onTarget();
#ifdef MASTER_SPIM
    void jobsCompleted(bool ok);
#endif

private:
#ifdef MASTER_SPIM
    Tasks *tasks;
    QList<PIDevice *> piDevList;
    QList<Cobolt *> laserList;
    QList<FilterWheel *> filterWheelList;
    QList<AA_MPDSnCxx *> aotfList;
#endif

    double exposureTime; // in ms
    double triggerRate;
    int binning = 1;

    QList<OrcaFlash *> camList;
    QList<SaveStackWorker *> ssWorkerList;
    QList<bool> enabledCameras;

    QStateMachine *sm = nullptr;

    SPIM_PI_DEVICES stackStage;
    QList<SPIM_PI_DEVICES> mosaicStages;
    QList<SPIM_PI_DEVICES> enabledMosaicStages;
    QMap<SPIM_PI_DEVICES, bool> enabledMosaicStageMap;

    int currentStep = 0;
    int totalSteps = 0;
    QMap<SPIM_PI_DEVICES, int> nSteps;
    QMap<SPIM_PI_DEVICES, int> currentSteps;
    QMap<SPIM_PI_DEVICES, QList<double> *> scanRangeMap;
    double scanVelocity = 1;

    QStringList outputPath;
    QString outputFname;
    QString runName;
    int frameCount;

    bool freeRun = true;
    bool capturing = false;
    bool _initialized = false;

    bool turnOffLasersAtEndOfAcquisition = false;

    int completedJobs;
    int successJobs;

    QMap<MACHINE_STATE, QState *> stateMap;

    void _setExposureTime(double expTime);
    void _startCapture();
    void setupStateMachine();

    void incrementCompleted(bool ok);

private slots:
    void onError(const QString &errMsg);
};

SPIM &spim();

#endif // SPIMHUB_H
