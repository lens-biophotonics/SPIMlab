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
#ifndef SPIM_NCORRGALVOS
#define SPIM_NCORRGALVOS 7
#endif

#define SPIM_RANGE_FROM_IDX 0
#define SPIM_RANGE_TO_IDX 1
#define SPIM_RANGE_STEP_IDX 2

#define SPIM_SCAN_DECIMALS 5

class SaveStackWorker;
class OrcaFlash;
class PIDevice;
class Cobolt;
class FilterWheel;
class AA_MPDSnCxx;
class Tasks;
namespace QtLab::hw::Thorlabs {
class MotorController;
};
class Autofocus;

enum SPIM_PI_DEVICES : int {
    PI_DEVICE_X_AXIS,
    PI_DEVICE_Y_AXIS,
    PI_DEVICE_Z_AXIS,
    PI_DEVICE_THETA_AXIS,
    PI_DEVICE_OBJ_AXIS,
};

enum SPIM_GALVOS : int {
    G2_x_AXIS1,
    G2_x_AXIS2, 
    G1_X_AXIS1,
    G1_X_AXIS2,
    G1_Y_AXIS1,
    G1_Y_AXIS2,
    G3_X_AXIS
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

    PIDevice *getPIDevice(const SPIM_PI_DEVICES dev) const;
    PIDevice *getPIDevice(const int dev) const;
    QList<PIDevice *> getPIDevices() const;

    galvoRamp *getCorrectionGalvo(int i) const; //returns a galvo of index i from the list
    QList<galvoRamp *> getCorrectionGalvos() const; //returns the entire list

    double getExposureTime() const;
    void setExposureTime(double ms);

    QList<Cobolt *> getLaserDevices() const;
    Cobolt *getLaser(const int n) const;

    QList<FilterWheel *> getFilterWheelDevices() const;
    FilterWheel *getFilterWheel(const int n) const;

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

    AA_MPDSnCxx *getAOTF(int dev);

    QString getRunName() const;
    void setRunName(const QString &value);

    QDir getFullOutputDir(int cam);

    Tasks *getTasks() const;
    Autofocus *getAutoFocus() const;

    void restartAutofocus();

    bool isMosaicStageEnabled(SPIM_PI_DEVICES dev) const;
    void setMosaicStageEnabled(SPIM_PI_DEVICES dev, bool enable);

    int getBinning() const;
    void setBinning(uint value);

    QtLab::hw::Thorlabs::MotorController *getMotorController() const;

public slots:
    void startFreeRun();
    void startAcquisition();
    void stop();
    void haltStages();
    void emergencyStop();
    void initialize();
    void uninitialize();

signals:
    void initialized() const;
    void captureStarted() const;
    void stopped() const;
    void jobsCompleted() const;
    void error(const QString) const;
    void onTarget();

private:
    Tasks *tasks;
    Autofocus *autoFocus;

    double exposureTime; // in ms
    double triggerRate;
    int binning = 1;

    QList<PIDevice *> piDevList;
    QList<galvoRamp *> correctionGalvos;
    QList<OrcaFlash *> camList;
    QList<SaveStackWorker *> ssWorkerList;
    QList<Cobolt *> laserList;
    QList<FilterWheel *> filterWheelList;
    QList<AA_MPDSnCxx *> aotfList;
    QtLab::hw::Thorlabs::MotorController *mc;

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
    QString runName;

    bool freeRun = true;
    bool capturing = false;

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
