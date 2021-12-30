#include <memory>
#include <cmath>

#include <QTimer>
#include <QFinalState>
#include <QHistoryState>

#include <qtlab/core/logger.h>
#include <qtlab/hw/hamamatsu/orcaflash.h>
#include <qtlab/hw/pi/pidevice.h>
#include <qtlab/hw/pi/pidaisychain.h>
#include <qtlab/hw/serial/serialport.h>
#include <qtlab/hw/serial/cobolt.h>
#include <qtlab/hw/serial/filterwheel.h>
#include <qtlab/hw/serial/AA_MPDSnCxx.h>

#include "spim.h"
#include "cameratrigger.h"
#include "tasks.h"
#include "galvoramp.h"
#include "savestackworker.h"

static Logger *logger = getLogger("SPIM");


SPIM::SPIM(QObject *parent) : QObject(parent)
{
    tasks = new Tasks(this);

    for (int i = 0; i < SPIM_NCAMS; ++i) {
        OrcaFlash *orca = new OrcaFlash(this);

        QThread *thread = new QThread();
        thread->setObjectName(QString("SaveStackWorker_thread_%1").arg(i));
        SaveStackWorker *ssWorker = new SaveStackWorker(orca);
        ssWorker->moveToThread(thread);
        thread->start();

        connect(ssWorker, &SaveStackWorker::error, this, &SPIM::onError);
        connect(ssWorker, &SaveStackWorker::captureCompleted, this, &SPIM::incrementCompleted);

        camList.insert(i, orca);
        ssWorkerList.insert(i, ssWorker);
        aotfList.insert(i, new AA_MPDSnCxx());
        filterWheelList.insert(i, new FilterWheel());
    }

    connect(tasks->getCameraTrigger(), &CameraTrigger::done, [ = ](){
        for (SaveStackWorker *ssWorker : ssWorkerList) {
            ssWorker->signalTriggerCompletion();
        }
        for (OrcaFlash *orca : camList) {
            orca->cap_stop();
        }
    });


    piDevList.reserve(SPIM_NPIDEVICES);
    piDevList.insert(PI_DEVICE_X_AXIS, new PIDevice("X axis", this));
    piDevList.insert(PI_DEVICE_Y_AXIS, new PIDevice("Y axis", this));
    piDevList.insert(PI_DEVICE_Z_AXIS, new PIDevice("Z axis", this));
    piDevList.insert(PI_DEVICE_LEFT_OBJ_AXIS,
                     new PIDevice("Left objective", this));
    piDevList.insert(PI_DEVICE_RIGHT_OBJ_AXIS,
                     new PIDevice("Right objective", this));
    for (PIDevice * dev : piDevList) {
        connect(dev, &PIDevice::connected, this, [ = ](){
            dev->setServoEnabled(true);
        });
    }

    for (int i = 0; i < SPIM_NPIDEVICES; ++i) {
        SPIM_PI_DEVICES dev = static_cast<SPIM_PI_DEVICES>(i);
        scanRangeMap.insert(dev, new QList<double>({0, 0, 0}));
        enabledMosaicStageMap[dev] = false;
    }

    PIDevice *xaxis = getPIDevice(PI_DEVICE_X_AXIS);
    connect(xaxis, &PIDevice::connected, this, [ = ](){
        xaxis->setTriggerOutput(PIDevice::OUTPUT_1, PIDevice::Axis, 1);
        xaxis->setTriggerOutput(
            PIDevice::OUTPUT_1, PIDevice::TriggerMode, PIDevice::InMotion);
        xaxis->setTriggerOutputEnabled(PIDevice::OUTPUT_1, true);
    });

    laserList.reserve(SPIM_NCOBOLT);
    for (int i = 0; i < SPIM_NCOBOLT; ++i) {
        laserList.insert(i, new Cobolt());
    }

    stackStage = PI_DEVICE_X_AXIS;
    mosaicStages << PI_DEVICE_Y_AXIS << PI_DEVICE_Z_AXIS;
    enabledMosaicStageMap[PI_DEVICE_Y_AXIS] = true;

    setupStateMachine();
}

SPIM::~SPIM()
{
}

void SPIM::initialize()
{
    try {
        logger->info("Initializing microscope");
        int nOfCameras = DCAM::init_dcam();
        if (nOfCameras < SPIM_NCAMS) {
            throw std::runtime_error(
                      QString("Found %1 of %2 cameras").arg(
                          nOfCameras).arg(SPIM_NCAMS).toStdString());
        }

        for (int i = 0; i < SPIM_NCAMS; ++i) {
            OrcaFlash *orca = camList.at(i);
            if (orca->isOpen()) {
                continue;
            }
            orca->open(i);
            orca->setSensorMode(OrcaFlash::SENSOR_MODE_PROGRESSIVE);
            orca->setTriggerSource(OrcaFlash::TRIGGERSOURCE_EXTERNAL);
            orca->setTriggerPolarity(OrcaFlash::POL_POSITIVE);
            orca->setOutputTrigger(OrcaFlash::OUTPUT_TRIGGER_KIND_PROGRAMMABLE,
                                   OrcaFlash::OUTPUT_TRIGGER_SOURCE_HSYNC,
                                   OrcaFlash::POL_POSITIVE,
                                   2e-6);
            orca->setPropertyValue(DCAM::DCAM_IDPROP_READOUT_DIRECTION,
                                   DCAM::DCAMPROP_READOUT_DIRECTION__FORWARD);
            orca->setPropertyValue(
                DCAM::DCAM_IDPROP_OUTPUTTRIGGER_PREHSYNCCOUNT, 0);
            orca->buf_alloc(4000);
            orca->logInfo();
        }

        for (int devnumber = 1; devnumber <= 16; ++devnumber) {
            for (PIDevice * dev : piDevList) {
                if (dev->getDeviceNumber() > devnumber) {
                    continue;
                }
                if (dev->isConnected()) {
                    continue;
                }
                if (dev->getPortName().isEmpty()) {
                    continue;
                }
                for (int i = 0; i < 5; ++i) {
                    try {
                        dev->connectDevice();
                        break;
                    }
                    catch (std::runtime_error) {
                        QString msg = "Cannot open device. Attempt %1 of 5";
                        msg = msg.arg(i + 1);
                        logger->warning(msg);
                        continue;
                    }
                }
            }
        }

        emit initialized();
        logger->info("Initialization completed");
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }
}

void SPIM::uninitialize()
{
    try {
        stop();
        closeAllDaisyChains();
        tasks->clearTasks();
        for (OrcaFlash * orca : camList) {
            if (orca->isOpen()) {
                orca->buf_release();
                orca->close();
            }
        }
        DCAM::uninit_dcam();
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }
}

bool SPIM::isMosaicStageEnabled(SPIM_PI_DEVICES dev) const
{
    return enabledMosaicStageMap[dev];
}

void SPIM::setMosaicStageEnabled(SPIM_PI_DEVICES dev, bool enable)
{
    enabledMosaicStageMap[dev] = enable;
}

Tasks *SPIM::getTasks() const
{
    return tasks;
}

QString SPIM::getRunName() const
{
    return runName;
}

void SPIM::setRunName(const QString &value)
{
    runName = value;
}

double SPIM::getScanVelocity() const
{
    return scanVelocity;
}

void SPIM::setScanVelocity(double value)
{
    scanVelocity = value;
}

AA_MPDSnCxx *SPIM::getAOTF(int dev)
{
    return aotfList.at(dev);
}

void SPIM::haltStages()
{
    for (PIDevice * dev : piDevList) {
        if (dev->isConnected()) {
            dev->halt();
        }
    }
}

void SPIM::emergencyStop()
{
    haltStages();
    stop();
}

double SPIM::getTriggerRate() const
{
    return triggerRate;
}

int SPIM::getNSteps(const SPIM_PI_DEVICES devEnum) const
{
    return nSteps[devEnum];
}

int SPIM::getTotalSteps() const
{
    return totalSteps;
}

int SPIM::getCurrentStep() const
{
    return currentStep;
}

QList<Cobolt *> SPIM::getLaserDevices() const
{
    return laserList;
}

Cobolt *SPIM::getLaser(const int n) const
{
    return laserList.at(n);
}

QList<FilterWheel *> SPIM::getFilterWheelDevices() const
{
    return filterWheelList;
}

FilterWheel *SPIM::getFilterWheel(const int n) const
{
    return filterWheelList.at(n);
}

double SPIM::getExposureTime() const
{
    return exposureTime;
}

void SPIM::setExposureTime(double ms)
{
    exposureTime = ms;
}

QList<OrcaFlash *> SPIM::getCameraDevices()
{
    return camList;
}

SaveStackWorker *SPIM::getSSWorker(int camNumber)
{
    return ssWorkerList.at(camNumber);
}

OrcaFlash *SPIM::getCamera(int camNumber) const
{
    return camList.at(camNumber);
}

PIDevice *SPIM::getPIDevice(const SPIM_PI_DEVICES dev) const
{
    return piDevList.value(dev);
}

PIDevice *SPIM::getPIDevice(const int dev) const
{
    return getPIDevice(static_cast<SPIM_PI_DEVICES>(dev));
}

QList<PIDevice *> SPIM::getPIDevices() const
{
    return piDevList;
}

void SPIM::startFreeRun()
{
    freeRun = true;
    logger->info("Start free run");
    _startCapture();
}

void SPIM::startAcquisition()
{
    freeRun = false;
    logger->info("Start acquisition");

    enabledMosaicStages.clear();
    for (const SPIM_PI_DEVICES d_enum : mosaicStages) {
        if (enabledMosaicStageMap[d_enum]) {
            enabledMosaicStages << d_enum;
        }
    }

    QList<SPIM_PI_DEVICES> stageEnumList;
    stageEnumList << enabledMosaicStages << stackStage;

    QList<PIDevice *> stageList;
    for (const SPIM_PI_DEVICES d_enum : stageEnumList) {
        stageList << getPIDevice(d_enum);

        int from = static_cast<int>(scanRangeMap[d_enum]->at(SPIM_RANGE_FROM_IDX) * pow(10, SPIM_SCAN_DECIMALS));
        int to = static_cast<int>(scanRangeMap[d_enum]->at(SPIM_RANGE_TO_IDX) * pow(10, SPIM_SCAN_DECIMALS));
        int step = static_cast<int>(scanRangeMap[d_enum]->at(SPIM_RANGE_STEP_IDX) * pow(10, SPIM_SCAN_DECIMALS));

        if (step == 0) {
            nSteps[d_enum] = 1;
        }
        else {
            nSteps[d_enum] = static_cast<int>(ceil((to - from) / step) + 1);
        }
    }

    totalSteps = 1;
    for (const SPIM_PI_DEVICES d_enum : enabledMosaicStages) {
        totalSteps *= nSteps[d_enum];
        currentSteps[d_enum] = 0;
    }
    logger->info(QString("Total number of stacks to acquire: %1 (with %2 frames in each)")
                 .arg(totalSteps).arg(nSteps[stackStage]));

    currentStep = 0;
    // create output directories
    for (int i = 0; i < SPIM_NCAMS; ++i) {
        getFullOutputDir(i).mkpath(".");
    }

    _startCapture();
}

void SPIM::_startCapture()
{
    capturing = true;

    try {
        _setExposureTime(exposureTime / 1000.);
    }
    catch (std::runtime_error e) {
        onError(e.what());
        return;
    }

    if (freeRun) {
        stateMap[STATE_CAPTURING]->setInitialState(stateMap[STATE_FREERUN]);
    }
    else {
        stateMap[STATE_CAPTURING]->setInitialState(stateMap[STATE_ACQUISITION]);
    }

    tasks->clearTasks();
    tasks->getCameraTrigger()->setFreeRunEnabled(freeRun);
    tasks->getCameraTrigger()->setNPulses(nSteps[stackStage]);

    emit captureStarted();
}

void SPIM::setupStateMachine()
{
    std::function<QState*(const MACHINE_STATE, QState *parent)> newState
        = [this](const MACHINE_STATE type, QState *parent = nullptr) {
        QState *state = new QState(parent);
        this->stateMap[type] = state;
        return state;
    };

    sm = new QStateMachine();

    QState *uninitState = newState(STATE_UNINITIALIZED, sm);
    sm->setInitialState(uninitState);

    QState *readyState = newState(STATE_READY, sm);
    QState *capturingState = newState(STATE_CAPTURING, sm);

    uninitState->addTransition(this, &SPIM::initialized, readyState);
    readyState->addTransition(this, &SPIM::captureStarted, capturingState);
    capturingState->addTransition(this, &SPIM::stopped, readyState);

    QHistoryState *historyState = new QHistoryState(sm);

    QState *errorState = newState(STATE_ERROR, sm);
    errorState->addTransition(historyState);

    sm->addTransition(this, &SPIM::error, errorState);

    /* free run */

    QState *freeRunState = newState(STATE_FREERUN, capturingState);
    connect(freeRunState, &QState::entered, this, [ = ](){
        try {
            for (OrcaFlash *orca : camList) {
                orca->cap_start();
            }

            tasks->start();
        }
        catch (std::runtime_error e) {
            onError(e.what());
        }
    });

    /* acquisition to file */

    QList<SPIM_PI_DEVICES> stageEnumList;
    stageEnumList << stackStage << mosaicStages;
    std::sort(stageEnumList.begin(), stageEnumList.end());

    QList<PIDevice *> stageList;
    for (const SPIM_PI_DEVICES d_enum : stageEnumList) {
        stageList << getPIDevice(d_enum);
    }

    QState *acquisitionState = newState(STATE_ACQUISITION, capturingState);

    QState *precaptureState = newState(STATE_PRECAPTURE, acquisitionState);
    QState *captureState = newState(STATE_CAPTURE, acquisitionState);

    acquisitionState->setInitialState(precaptureState);

    precaptureState->setChildMode(QState::ParallelStates);
    captureState->setChildMode(QState::ParallelStates);

    precaptureState->addTransition(precaptureState, &QState::finished, captureState);
    captureState->addTransition(this, &SPIM::jobsCompleted, precaptureState);

    // setup parallel states in precaptureState
    QState *pollingState = new QState(precaptureState);
    QState *pollingInProgressState = new QState(pollingState);
    QFinalState *pollingDone = new QFinalState(pollingState);

    pollingState->setInitialState(pollingInProgressState);
    pollingInProgressState->addTransition(this, &SPIM::onTarget, pollingDone);


    // setup parallel states in captureState
    for (SaveStackWorker *ssWorker : ssWorkerList) {
        QState *camState = new QState(captureState);
        QState *camBusyState = new QState(camState);
        QFinalState *finalState = new QFinalState(camState);

        camState->setInitialState(camBusyState);
        camBusyState->addTransition(ssWorker, &SaveStackWorker::captureCompleted, finalState);
    }

    // polling timer used to check when stages have reached target
    QTimer *pollTimer = new QTimer(this);
    connect(pollTimer, &QTimer::timeout, this, [ = ](){
        QList<SPIM_PI_DEVICES> myStageEnumList;
        myStageEnumList << enabledMosaicStages << stackStage;
        try {
            for (const SPIM_PI_DEVICES d_enum : myStageEnumList) {
                if (!getPIDevice(d_enum)->isOnTarget()) {
                    return;
                }
            }
        }
        catch (std::runtime_error e) {
            onError(e.what());
        }

        emit onTarget();
    });

    connect(acquisitionState, &QState::exited, this, [ = ](){
        pollTimer->stop();
        haltStages();
    });

    connect(precaptureState, &QState::entered, this, [ = ](){
        if (!capturing) {
            return;
        }
        tasks->stop();
        completedJobs = successJobs = 0;

        // compute target position
        QMap<SPIM_PI_DEVICES, double> targetPositions;
        targetPositions[stackStage]
            = scanRangeMap[stackStage]->at(SPIM_RANGE_FROM_IDX);
        QList<SPIM_PI_DEVICES> myStageEnumList;
        myStageEnumList << enabledMosaicStages << stackStage;
        for (const SPIM_PI_DEVICES d_enum : myStageEnumList) {
            double from = scanRangeMap[d_enum]->at(SPIM_RANGE_FROM_IDX);
            double step = scanRangeMap[d_enum]->at(SPIM_RANGE_STEP_IDX);
            targetPositions[d_enum] = from + currentSteps[d_enum] * step;
        }

        try {
            // move stages to target position
            for (SPIM_PI_DEVICES d_enum : myStageEnumList) {
                PIDevice *dev = getPIDevice(d_enum);
                dev->setVelocity(scanVelocity);

                double pos = targetPositions[d_enum];
                logger->info(QString("Moving %1 to %2")
                             .arg(dev->getVerboseName())
                             .arg(pos));
                dev->move(pos);
            }

            QString fname;
            QStringList axis = {"x_", "y_", "z_"};
            QStringList side = {"l", "r"};
            int k = 0;
            for (SPIM_PI_DEVICES d_enum : stageEnumList) {
                double pos = targetPositions[d_enum];
                fname += axis.at(k) + QString("%1").arg(pos, (4 + SPIM_SCAN_DECIMALS), 'f', SPIM_SCAN_DECIMALS, '0');
                k += 1;
                fname += "_";
            }

            // prepare and start acquisition thread
            for (int i = 0; i < SPIM_NCAMS; ++i) {
                SaveStackWorker *ssWorker = ssWorkerList.at(i);
                ssWorker->setTimeout(2 * 1e6 / getTriggerRate());
                ssWorker->setOutputPath(getFullOutputDir(i).absolutePath());
                ssWorker->setOutputFileName(fname + "_cam_" + side.at(i));
                ssWorker->setFrameCount(nSteps[stackStage]);
            }
        } catch (std::runtime_error e) {
            onError(e.what());
            return;
        }

        pollTimer->start(200);
    });

    // when stage is on target: start cameras, galvos and trigger
    // then start moving the stack stage
    connect(captureState, &QState::entered, this, [ = ] {
        pollTimer->stop();

        try {
            // move stack axis to end position
            double stackTo = scanRangeMap[stackStage]->at(SPIM_RANGE_TO_IDX);
            double stackStep = scanRangeMap[stackStage]->at(SPIM_RANGE_STEP_IDX);

            PIDevice *dev = getPIDevice(stackStage);
            dev->setVelocity(triggerRate * stackStep);
            logger->info(QString("Start acquisition of stack: %1/%2")
                         .arg(currentStep + 1).arg(totalSteps));
            logger->info(QString("Moving %1 to %2")
                         .arg(dev->getVerboseName())
                         .arg(stackTo));

            for (int i = 0; i < SPIM_NCAMS; ++i) {
                camList.at(i)->cap_start();
                QMetaObject::invokeMethod(ssWorkerList.at(i), &SaveStackWorker::start);
            }
            tasks->start();
            dev->move(stackTo);
        } catch (std::runtime_error e) {
            onError(e.what());
            return;
        }
    }, Qt::QueuedConnection);

    sm->start();
}

void SPIM::stop()
{
    if (!capturing) {
        return;
    }
    logger->info("Stop");
    capturing = false;
    try {
        for (SaveStackWorker *ssWorker : ssWorkerList) {
            ssWorker->stop();
        }
        for (OrcaFlash * orca : camList) {
            if (orca->isOpen()) {
                orca->cap_stop();
            }
        }
        tasks->clearTasks();
    } catch (std::runtime_error e) {
        emit error(e.what());
        return;
    }

    emit stopped();
}

void SPIM::_setExposureTime(double expTime)
{
    double lineInterval = -1;
    int nOfLines = -1;

    try {
        for (int i = 0; i < camList.count(); ++i) {
            OrcaFlash *orca = camList.at(i);
            expTime = orca->setGetExposureTime(expTime);

            double tempDouble = orca->getLineInterval();
            int tempInt = orca->nOfLines();

            if (i > 0) {
                if (fabs(tempDouble - lineInterval) > 0.001 || tempInt != nOfLines) {
                    QString m("Different values for line interval and number of "
                              "lines: Cam 0: %1 %2; Cam %3: %4 %5");
                    m = m.arg(lineInterval).arg(nOfLines).arg(i).arg(tempDouble)
                        .arg(tempInt);
                    logger->warning(m);
                }
            }
            else {
                lineInterval = tempDouble;
                nOfLines = tempInt;
            }
        }

        double sampRate = 1 / lineInterval;

        double frameRate = 1 / (expTime + (nOfLines + 10) * lineInterval);
        double fraction = 0.95;
        triggerRate = fraction * frameRate;

        logger->info(QString("Exposure time: %1 ms").arg(expTime * 1000));
        logger->info(QString("Line interval: %1 us").arg(lineInterval));
        logger->info(QString("Achievable frame rate: %1 Hz").arg(frameRate));
        logger->info(QString("Acquisition rate: %1 Hz").arg(triggerRate));

        uint64_t nSamples = static_cast<uint64_t>(sampRate / triggerRate);

        tasks->getCameraTrigger()->setPulseFreq(sampRate / nSamples);

        tasks->getGalvoRamp()->setSampleRate(sampRate);
        tasks->getGalvoRamp()->setSampsPerChan(nSamples);
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }
}

void SPIM::incrementCompleted(bool ok)
{
    if (freeRun) {
        return;
    }
    if (ok) {
        ++successJobs;
    }
    if (successJobs == 1) {
        tasks->stop();
    }
    if (++completedJobs == SPIM_NCAMS) {
        if (successJobs == SPIM_NCAMS)  {
            currentStep++;

            // check exit condition
            if (currentStep >= totalSteps) {
                logger->info("Acquisition completed");
                stop();
                return;
            }

            SPIM_PI_DEVICES xAxis = enabledMosaicStages.at(0);

            int newX = currentSteps[xAxis] + 1;
            if (newX >= nSteps[xAxis]) {
                newX = 0;
                if (enabledMosaicStages.size() > 1) {
                    SPIM_PI_DEVICES yAxis = enabledMosaicStages.at(1);
                    currentSteps[yAxis]++;
                }
            }
            currentSteps[xAxis] = newX;
        }
        else if (capturing) {  // if not stopped
            logger->warning(QString("Re-acquiring stack: %1/%2")
                            .arg(currentStep + 1).arg(totalSteps));
        }
        logger->info(QString("Success jobs: %1/%2").arg(successJobs).arg(SPIM_NCAMS));
        emit jobsCompleted();
    }
}

QStringList SPIM::getOutputPathList() const
{
    return outputPath;
}

void SPIM::setOutputPathList(const QStringList &sl)
{
    outputPath = sl;
}

QState *SPIM::getState(const SPIM::MACHINE_STATE stateEnum)
{
    return stateMap[stateEnum];
}

QList<SPIM_PI_DEVICES> SPIM::getMosaicStages() const
{
    return mosaicStages;
}

QList<double> *SPIM::getScanRange(const SPIM_PI_DEVICES dev) const
{
    return scanRangeMap[dev];
}

SPIM_PI_DEVICES SPIM::getStackStage() const
{
    return stackStage;
}

void SPIM::onError(const QString &errMsg)
{
    stop();
    emit error(errMsg);
}

QDir SPIM::getFullOutputDir(int cam)
{
    return QDir::cleanPath(outputPath.at(cam) + QDir::separator() + runName);
}

SPIM &spim()
{
    static auto instance = std::make_unique<SPIM>();
    return *instance;
}
