#include <memory>
#include <cmath>

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

    piDevList.reserve(5);
    piDevList.insert(PI_DEVICE_X_AXIS, new PIDevice("X axis", this));
    piDevList.insert(PI_DEVICE_Y_AXIS, new PIDevice("Y axis", this));
    piDevList.insert(PI_DEVICE_Z_AXIS, new PIDevice("Z axis", this));
    piDevList.insert(PI_DEVICE_LEFT_OBJ_AXIS,
                     new PIDevice("Left objective", this));
    piDevList.insert(PI_DEVICE_RIGHT_OBJ_AXIS,
                     new PIDevice("Right objective", this));

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
                dev->setServoEnabled(true);
            }
        }
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }

    emit initialized();
}

void SPIM::uninitialize()
{
    try {
        for (QThread *t: acqThreads) {
            t->requestInterruption();
        }
        for (QThread *t: acqThreads) {
            t->wait(1000);
        }
        closeAllDaisyChains();
        qDeleteAll(camList);
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
    startCapture(true);
}

void SPIM::startAcquisition()
{
    startCapture(false);
}

void SPIM::startCapture(bool freeRun)
{
    logger->info("Start acquisition");

    try {
        _setExposureTime(exposureTime / 1000.);

        int i = 0;
        for (OrcaFlash *orca : camList) {
            if (freeRun) {
                orca->setNFramesInBuffer(10);
            }
            else {
                SaveStackWorker *acqThread = new SaveStackWorker(orca);
                acqThread->setOutputFileName(QString("output%1.raw").arg(i++));
                acqThread->setFrameCount(100);
                acqThreads.append(acqThread);

                connect(acqThread, &SaveStackWorker::error,
                        this, &SPIM::onError);
                connect(acqThread, &QThread::finished,
                        acqThread, &QThread::deleteLater);
                connect(orca, &OrcaFlash::captureStarted, acqThread, [ = ](){
                    acqThread->start();
                });

                orca->setNFramesInBuffer(100);  //FIXME
            }

            orca->startCapture();
        }

        cameraTrigger->setFreeRunEnabled(true); //FIXME
        galvoRamp->start();

        cameraTrigger->start();
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }

    emit captureStarted();
}

void SPIM::stop()
{
    try {
        for (QThread *acqThread : acqThreads) {
            if (acqThread->isRunning()) {
                acqThread->requestInterruption();
            }
        }
        acqThreads.clear();
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
        double freq = 0.98 * frameRate;

        galvoRamp->setSampleRate(sampRate);
        cameraTrigger->setSampleRate(sampRate);

        uint64_t nSamples = static_cast<uint64_t>(sampRate / freq);
        galvoRamp->setNSamples(nSamples);
        cameraTrigger->setNSamples(nSamples);

        galvoRamp->setNRamp(0, nOfLines);
        galvoRamp->setNRamp(1, nOfLines);
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }
}

void SPIM::onError(const QString &errMsg)
{
    stop();
    emit error(errMsg);
}

SPIM &spim()
{
    static auto instance = std::make_unique<SPIM>(nullptr);
    return *instance;
}
