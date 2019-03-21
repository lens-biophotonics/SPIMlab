#include <memory>
#include <cmath>

#include "spim.h"
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
            orca->open(i);
            orca->setSensorMode(OrcaFlash::SENSOR_MODE_PROGRESSIVE);
            orca->setTriggerSource(OrcaFlash::TRIGGERSOURCE_EXTERNAL);
            orca->setOutputTrigger(OrcaFlash::OUTPUT_TRIGGER_KIND_PROGRAMMABLE,
                                   OrcaFlash::OUTPUT_TRIGGER_SOURCE_HSYNC,
                                   2e-6);
            orca->setPropertyValue(DCAM::DCAM_IDPROP_READOUT_DIRECTION,
                                   DCAM::DCAMPROP_READOUT_DIRECTION__FORWARD);
        }

        for (PIDevice * dev : piDevList) {
            if (dev->getPortName().isEmpty()) {
                continue;
            }
            dev->connectDevice();
            dev->setServoEnabled(true);
        }
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }

    setupGalvoRampTriggerSource(cameraTrigger->getTerms());

    emit initialized();
}

void SPIM::uninitialize()
{
    try {
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

void SPIM::setupGalvoRampTriggerSource(const QStringList &terminals)
{
    try {
        galvoRamp->setTriggerSource(terminals.at(0));
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }
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
    try {
        _setExposureTime(exposureTime / 1000.);
        for (OrcaFlash * orca : camList) {
            orca->setNFramesInBuffer(10);
            orca->startCapture();
        }

        cameraTrigger->setFreeRunEnabled(true);
        galvoRamp->start();

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
        cameraTrigger->setFreeRunEnabled(false);
        acqThread = new QThread();
        worker = new SaveStackWorker();
        worker->setOutputFileName("output.raw");
        worker->setFrameCount(100);
        worker->moveToThread(acqThread);

        connect(acqThread, &QThread::started, worker, &SaveStackWorker::saveToFile);
        connect(worker, &SaveStackWorker::finished, acqThread, &QThread::quit);
        connect(worker, &SaveStackWorker::finished, worker, &SaveStackWorker::deleteLater);
        connect(worker, &SaveStackWorker::finished, this, &SPIM::stop);
        connect(worker, &SaveStackWorker::error, this, &SPIM::onError);
        connect(acqThread, &QThread::finished, acqThread, &QThread::deleteLater);

//        orca->setNFramesInBuffer(100);  //FIXME
//        orca->startCapture();

        acqThread->start();
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }

    emit captureStarted();
}

void SPIM::stop()
{
    try {
        if (acqThread && acqThread->isRunning()) {
            acqThread->requestInterruption();
            acqThread = nullptr;
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

        uint64_t nSamples = static_cast<uint64_t>(round(expTime / lineInterval + nOfLines));
        double rate = 1 / lineInterval;

        galvoRamp->setSampleRate(rate);
        galvoRamp->setNSamples(nSamples);

        galvoRamp->setNRamp(0, nOfLines);
        galvoRamp->setNRamp(1, nOfLines);

        double frameRate = 1 / (expTime + (nOfLines + 10) * lineInterval);
        double freq = 0.98 * frameRate;
        cameraTrigger->setFrequency(freq);
        cameraTrigger->setInitialDelays({0, 0.5 / freq});
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
