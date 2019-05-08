#include <memory>
#include <cmath>

#include <QTimer>
#include <QFinalState>
#include <QDir>

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
        scanRangeMap.insert(static_cast<PI_DEVICES>(i),
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
    mosaicStages << PI_DEVICE_Z_AXIS << PI_DEVICE_Y_AXIS;

    QList<PI_DEVICES> allStages;
    allStages << mosaicStages << stackStage;
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

PIDevice *SPIM::getPIDevice(const SPIM::PI_DEVICES dev) const
{
    return piDevList.value(dev);
}

PIDevice *SPIM::getPIDevice(const int dev) const
{
    return getPIDevice(static_cast<PI_DEVICES>(dev));
}

QList<PIDevice *> SPIM::getPIDevices() const
{
    return piDevList;
}

void SPIM::startFreeRun()
{
    try{
        _setExposureTime(exposureTime / 1000.);
        for (OrcaFlash *orca : camList) {
            orca->setNFramesInBuffer(10);
            orca->startCapture();
        }
        galvoRamp->start();

        cameraTrigger->setFreeRunEnabled(true);
        cameraTrigger->start();
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }

    emit captureStarted();
}

void SPIM::startAcquisition()
{
    logger->info("Start acquisition");

    try {
        _setExposureTime(exposureTime / 1000.);
    }
    catch (std::runtime_error e) {
        onError(e.what());
        return;
    }

    QList<PI_DEVICES> stageEnumList;
    stageEnumList << mosaicStages << stackStage;

    QMap<PI_DEVICES, int> nSteps;

    QList<PIDevice *> stageList;
    for (const PI_DEVICES d_enum : stageEnumList) {
        stageList << getPIDevice(d_enum);

        double from = scanRangeMap[d_enum]->at(SPIM_RANGE_FROM_IDX);
        double to = scanRangeMap[d_enum]->at(SPIM_RANGE_TO_IDX);
        double step = scanRangeMap[d_enum]->at(SPIM_RANGE_STEP_IDX);

        if (step == 0.) {
            nSteps[d_enum] = 1;
        }
        else {
            nSteps[d_enum] = static_cast<int>(ceil((to - from) / step)) + 1;
        }
    }

    QStringList axes;
    try {
        for (PIDevice *dev : stageList) {
            QString axis = dev->getAxisIdentifiers().at(0);
            axes << axis;
        }
    }
    catch (std::runtime_error e) {
        onError(e.what());
        return;
    }

    if (sm != nullptr) {
        delete sm;
    }
    sm = new QStateMachine(this);
    QState *precaptureState = new QState(QState::ParallelStates, sm);
    sm->setInitialState(precaptureState);

    QState *captureState = new QState(QState::ParallelStates, sm);
    precaptureState->addTransition(cameraTrigger,
                                   &CameraTrigger::started, captureState);
    captureState->addTransition(captureState,
                                &QState::finished, precaptureState);

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

        orca->buf_release();
        orca->buf_alloc(100);      //FIXME
    }

    // polling timer used to check when stages have reached targed
    QTimer *pollTimer = new QTimer(sm);
    connect(pollTimer, &QTimer::timeout, [ = ](){
        int i = 0;
        for (PIDevice *dev : stageList) {
            dev->isOnTarget(axes.at(i++)); // will emit PIDevice::onTarget
        }
    });

    int totalSteps = 1;
    for (const PI_DEVICES d_enum : mosaicStages) {
        totalSteps *= nSteps[d_enum];
    }
    double stackFrom = scanRangeMap[stackStage]->at(SPIM_RANGE_FROM_IDX);
    double stackStep = scanRangeMap[stackStage]->at(SPIM_RANGE_STEP_IDX);

    connect(precaptureState, &QState::entered, this, [ = ](){
        galvoRamp->stop();
        cameraTrigger->stop();

        // check exit condition
        if (currentStep >= totalSteps) {
            stop();
            return;
        }

        try {
            const double vel = 1;
            getPIDevice(stackStage)->setVelocities(axes.at(0), &vel);

            QList<double> targetPositions;
            for (const PI_DEVICES d_enum : mosaicStages) {
                double from = scanRangeMap[d_enum]->at(SPIM_RANGE_FROM_IDX);
                double step = scanRangeMap[d_enum]->at(SPIM_RANGE_STEP_IDX);
                targetPositions << from + currentStep * step;
            }
            targetPositions << stackFrom;

            int i = 0;
            for (PIDevice *dev : stageList) {
                double pos = targetPositions.at(i);
                logger->info(QString("Moving %1 to %2")
                             .arg(dev->getVerboseName())
                             .arg(pos));
                dev->move(axes.at(i++), &pos);
            }
        }
        catch (std::runtime_error e) {
            onError(e.what());
            return;
        }

        pollTimer->start(200);
    });

    // when stage is on target: start cameras, galvos and trigger
    connect(precaptureState, &QState::finished, this, [ = ] {
        pollTimer->stop();

        try {
            for (OrcaFlash *orca : camList) {
                orca->cap_start();

                SaveStackWorker *acqThread = new SaveStackWorker(orca);
                QString fname;
                for (PIDevice *dev: stageList) {
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

            cameraTrigger->setFreeRunEnabled(false);
            cameraTrigger->start();

            const double vel = triggerRate * stackStep;
            getPIDevice(stackStage)->setVelocities(axes.at(0), &vel);
            getPIDevice(stackStage)->move(axes.last(), &stackFrom);
        } catch (std::runtime_error e) {
            onError(e.what());
            return;
        }
    }, Qt::QueuedConnection);

    connect(captureState, &QState::finished, this, [ = ] {
        currentStep++;
    });

    currentStep = 0;
    QDir(outputPath).mkpath(".");
    sm->start();

    try {
        for (int i = 0; i < stageList.size(); i++) {
            QString axis = axes.at(i);
            double vel = 1.;
            stageList.at(i)->setVelocities(axis, &vel);
        }
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }

    emit captureStarted();
}

void SPIM::stop()
{
    logger->info("stop");
    try {
        if (sm != nullptr) {
            sm->stop();
            sm->deleteLater();
            sm = nullptr;
        }
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

QList<SPIM::PI_DEVICES> SPIM::getMosaicStages() const
{
    return mosaicStages;
}

QList<double> *SPIM::getScanRange(const SPIM::PI_DEVICES dev) const
{
    return scanRangeMap[dev];
}

SPIM::PI_DEVICES SPIM::getStackStage() const
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
