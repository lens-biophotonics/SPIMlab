#include <memory>
#include <cmath>

#include <QTimer>
#include <QFinalState>
#include <QDir>
#include <QHistoryState>

#include "spim.h"
#include "orcaflash.h"
#include "cameratrigger.h"
#include "galvoramp.h"
#include "pidevice.h"
#include "cobolt.h"
#include "serialport.h"
#include "savestackworker.h"
#include "logger.h"

#include "pidaisychain.h"

static Logger *logger = getLogger("SPIM");


SPIM::SPIM(QObject *parent) : QObject(parent)
{
    for (int i = 0; i < SPIM_NCAMS; ++i) {
        camList.insert(i, new OrcaFlash());
    }

    cameraTrigger = new CameraTrigger();
    galvoRamp = new GalvoRamp();

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
        scanRangeMap.insert(static_cast<SPIM_PI_DEVICES>(i),
                            new QList<double>({0, 0, 0}));
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
    mosaicStages << PI_DEVICE_Y_AXIS;

    setupStateMachine();
}

SPIM::~SPIM()
{
}

void SPIM::initialize()
{
    try {
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
            orca->setOutputTrigger(OrcaFlash::OUTPUT_TRIGGER_KIND_PROGRAMMABLE,
                                   OrcaFlash::OUTPUT_TRIGGER_SOURCE_HSYNC,
                                   2e-6);
            orca->setPropertyValue(DCAM::DCAM_IDPROP_READOUT_DIRECTION,
                                   DCAM::DCAMPROP_READOUT_DIRECTION__FORWARD);
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
        delete galvoRamp;
        delete cameraTrigger;
        DCAM::uninit_dcam();
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }
}

double SPIM::getScanVelocity() const
{
    return scanVelocity;
}

void SPIM::setScanVelocity(double value)
{
    scanVelocity = value;
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

double SPIM::getExposureTime() const
{
    return exposureTime;
}

void SPIM::setExposureTime(double ms)
{
    exposureTime = ms;
}

GalvoRamp *SPIM::getGalvoRamp() const
{
    return galvoRamp;
}

CameraTrigger *SPIM::getCameraTrigger() const
{
    return cameraTrigger;
}

OrcaFlash *SPIM::getCamera(int camNumber) const
{
    return camList.at(camNumber);
}

QList<OrcaFlash *> SPIM::getCameras()
{
    return camList;
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
    _startAcquisition();
}

void SPIM::startAcquisition()
{
    freeRun = false;
    _startAcquisition();
}

void SPIM::_startAcquisition()
{
    logger->info("Start acquisition");

    try {
        _setExposureTime(exposureTime / 1000.);
    }
    catch (std::runtime_error e) {
        onError(e.what());
        return;
    }

    cameraTrigger->setFreeRunEnabled(freeRun);

    if (freeRun) {
        stateMap[STATE_CAPTURING]->setInitialState(stateMap[STATE_FREERUN]);
    }
    else {
        stateMap[STATE_CAPTURING]->setInitialState(stateMap[STATE_ACQUISITION]);

        QList<SPIM_PI_DEVICES> stageEnumList;
        stageEnumList << mosaicStages << stackStage;

        QList<PIDevice *> stageList;
        for (const SPIM_PI_DEVICES d_enum : stageEnumList) {
            stageList << getPIDevice(d_enum);

            double from = scanRangeMap[d_enum]->at(SPIM_RANGE_FROM_IDX);
            double to = scanRangeMap[d_enum]->at(SPIM_RANGE_TO_IDX);
            double step = scanRangeMap[d_enum]->at(SPIM_RANGE_STEP_IDX);

            if (step == 0.) {
                nSteps[d_enum] = 1;
            }
            else {
                nSteps[d_enum] = static_cast<int>(ceil((to - from) / step));
            }
        }

        totalSteps = 1;
        for (const SPIM_PI_DEVICES d_enum : mosaicStages) {
            nSteps[d_enum] += 1;
            totalSteps *= nSteps[d_enum];
        }

        currentStep = 0;
        QDir(outputPath).mkpath(".");
    }
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
                orca->setNFramesInBuffer(10);
                orca->startCapture();
            }
            galvoRamp->start();
            cameraTrigger->start();
        }
        catch (std::runtime_error e) {
            onError(e.what());
        }
    });

    /* acquisition to file */

    QList<SPIM_PI_DEVICES> stageEnumList;
    stageEnumList << stackStage << mosaicStages;

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

    precaptureState->addTransition(precaptureState, &QState::finished,
                                   captureState);
    captureState->addTransition(captureState, &QState::finished,
                                precaptureState);

    // setup parallel states in precaptureState
    for (PIDevice *dev : stageList) {
        QState *pollingState = new QState(precaptureState);
        QState *pollingInProgressState = new QState(pollingState);
        QFinalState *pollingDone = new QFinalState(pollingState);

        pollingState->setInitialState(pollingInProgressState);
        pollingInProgressState->addTransition(
            dev, &PIDevice::onTarget, pollingDone);
    }

    // setup parallel states in captureState
    for (OrcaFlash *orca : camList) {
        QState *camState = new QState(captureState);
        QState *camBusyState = new QState(camState);
        QFinalState *finalState = new QFinalState(camState);

        camState->setInitialState(camBusyState);
        camBusyState->addTransition(orca, &OrcaFlash::stopped, finalState);
    }

    // polling timer used to check when stages have reached targed
    QTimer *pollTimer = new QTimer(this);
    connect(pollTimer, &QTimer::timeout, this, [ = ](){
        try {
            for (PIDevice *dev : stageList) {
                dev->isOnTarget(); // will emit PIDevice::onTarget
            }
        }
        catch (std::runtime_error e) {
            onError(e.what());
        }
    });

    connect(acquisitionState, &QState::entered, this, [ = ](){
        for (OrcaFlash *orca : camList) {
            orca->buf_release();
            orca->buf_alloc(100);      //FIXME
        }
    });

    connect(acquisitionState, &QState::exited, this, [ = ](){
        pollTimer->stop();
    });

    connect(precaptureState, &QState::entered, this, [ = ](){
        galvoRamp->stop();
        cameraTrigger->stop();

        // check exit condition
        if (currentStep >= totalSteps) {
            stop();
            return;
        }

        // go to target position
        QMap<SPIM_PI_DEVICES, double> targetPositions;
        targetPositions[stackStage]
            = scanRangeMap[stackStage]->at(SPIM_RANGE_FROM_IDX);
        for (const SPIM_PI_DEVICES d_enum : mosaicStages) {
            double from = scanRangeMap[d_enum]->at(SPIM_RANGE_FROM_IDX);
            double step = scanRangeMap[d_enum]->at(SPIM_RANGE_STEP_IDX);
            targetPositions[d_enum] = from + currentStep * step;
        }

        try {
            for (SPIM_PI_DEVICES d_enum : stageEnumList) {
                PIDevice *dev = getPIDevice(d_enum);
                dev->setVelocity(scanVelocity);

                double pos = targetPositions[d_enum];
                logger->info(QString("Moving %1 to %2")
                             .arg(dev->getVerboseName())
                             .arg(pos));
                dev->move(pos);
            }
        }
        catch (std::runtime_error e) {
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
            for (OrcaFlash *orca : camList) {
                orca->cap_start();

                SaveStackWorker *acqThread = new SaveStackWorker(orca);
                QString fname;
                QList<PIDevice *> tempStageList;
                tempStageList << getPIDevice(stackStage);
                for (const SPIM_PI_DEVICES d_enum : mosaicStages) {
                    tempStageList << getPIDevice(d_enum);
                }
                for (PIDevice *dev: tempStageList) {
                    double pos = dev->getCommandedPosition().at(0);
                    fname += QString("%1_").arg(pos);
                }
                fname += QString("cam_%1").arg(orca->getCameraIndex());

                acqThread->setOutputFileName(QDir(outputPath).filePath(fname));

                acqThread->setFrameCount(nSteps[stackStage]);

                connect(acqThread, &QThread::finished,
                        acqThread, &QThread::deleteLater);

                connect(acqThread, &SaveStackWorker::error,
                        this, &SPIM::onError);

                acqThread->start();
            }

            galvoRamp->start();
            cameraTrigger->start();

            // move stack axis to end position
            double stackTo = scanRangeMap[stackStage]->at(SPIM_RANGE_TO_IDX);
            double stackStep = scanRangeMap[stackStage]->at(SPIM_RANGE_STEP_IDX);

            PIDevice *dev = getPIDevice(stackStage);
            dev->setVelocity(triggerRate * stackStep);
            logger->info(QString("Moving %1 to %2")
                         .arg(dev->getVerboseName())
                         .arg(stackTo));
            dev->move(stackTo);
        } catch (std::runtime_error e) {
            onError(e.what());
            return;
        }
    }, Qt::QueuedConnection);

    connect(captureState, &QState::finished, this, [ = ] {
        currentStep++;
    });

    sm->start();
}

void SPIM::stop()
{
    logger->info("stop");
    try {
        for (PIDevice * dev : piDevList) {
            if (dev->isConnected()) {
                dev->halt();
            }
        }
        for (OrcaFlash * orca : camList) {
            orca->stop();
        }
        galvoRamp->stop();
        cameraTrigger->stop();
    } catch (std::runtime_error e) {
        onError(e.what());
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
        triggerRate = 0.98 * frameRate;

        galvoRamp->setSampleRate(sampRate);
        cameraTrigger->setSampleRate(sampRate);

        uint64_t nSamples = static_cast<uint64_t>(sampRate / triggerRate);
        galvoRamp->setNSamples(nSamples);
        cameraTrigger->setNSamples(nSamples);

        galvoRamp->setNRamp(0, nOfLines);
        galvoRamp->setNRamp(1, nOfLines);
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }
}

QString SPIM::getOutputPath() const
{
    return outputPath;
}

void SPIM::setOutputPath(const QString &value)
{
    outputPath = value;
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

SPIM &spim()
{
    static auto instance = std::make_unique<SPIM>();
    return *instance;
}
