#include <memory>
#include <cmath>

#include "spim.h"
#include "logger.h"

#include "pidaisychain.h"

#define NCAMS 2
#define NGALVORAMPS 2

static Logger *logger = getLogger("SPIM");


SPIM::SPIM(QObject *parent) : QObject(parent)
{
    for (int i = 0; i < NCAMS; ++i) {
        camList.insert(i, new OrcaFlash());
    }

    cameraTrigger = new CameraTrigger(this);

    for (int i = 0; i < NGALVORAMPS; ++i) {
        galvoList.insert(i, new GalvoRamp(this));
    }

    piDevList.reserve(5);
    piDevList.insert(PI_DEVICE_X_AXIS, new PIDevice(this));
    piDevList.insert(PI_DEVICE_Y_AXIS, new PIDevice(this));
    piDevList.insert(PI_DEVICE_Z_AXIS, new PIDevice(this));
    piDevList.insert(PI_DEVICE_LEFT_OBJ_AXIS, new PIDevice(this));
    piDevList.insert(PI_DEVICE_RIGHT_OBJ_AXIS, new PIDevice(this));
}

SPIM::~SPIM()
{
}

void SPIM::initialize()
{
    try {
        DCAM::init_dcam();

        for (int i = 0; i < NCAMS; ++i) {
            OrcaFlash *orca = camList.at(i);
            orca->open(i);
            orca->setSensorMode(OrcaFlash::SENSOR_MODE_PROGRESSIVE);
            orca->setGetTriggerMode(OrcaFlash::TRIGMODE_EDGE);
            orca->setOutputTrigger(OrcaFlash::OUTPUT_TRIGGER_KIND_PROGRAMMABLE,
                                   OrcaFlash::OUTPUT_TRIGGER_SOURCE_VSYNC);
        }

        foreach (PIDevice * dev, piDevList) {
            dev->connectDevice();
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
        closeAllDaisyChains();
        qDeleteAll(camList);
        DCAM::uninit_dcam();
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }
}

GalvoRamp *SPIM::getGalvoRamp(int number) const
{
    return galvoList.at(number);
}

CameraTrigger *SPIM::getCameraTrigger() const
{
    return cameraTrigger;
}

OrcaFlash *SPIM::getCamera(int camNumber) const
{
    return camList.at(camNumber);
}

void SPIM::setupCameraTrigger(
    const QStringList &COPhysicalChans, const QStringList &terminals)
{
    try {
        cameraTrigger->setPhysicalChannels(COPhysicalChans);
        cameraTrigger->setTerms(terminals);

        for (int i = 0; i < NGALVORAMPS; ++i) {
            galvoList.at(i)->setTriggerSource(terminals.at(i));
        }
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }
}

PIDevice *SPIM::piDevice(const SPIM::PI_DEVICES dev) const
{
    return piDevList.value(dev);
}

void SPIM::startFreeRun()
{
    try {
        setExposureTime(0.0001);
        foreach(OrcaFlash * orca, camList) {
            orca->setNFramesInBuffer(10);
            orca->startCapture();
        }

        cameraTrigger->setFreeRunEnabled(true);

        foreach(GalvoRamp * galvoRamp, galvoList) {
            galvoRamp->start();
        }

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
        thread = new QThread();
        worker = new SaveStackWorker();
        worker->setOutputFileName("output.raw");
        worker->setFrameCount(100);
        worker->moveToThread(thread);

        connect(thread, &QThread::started, worker, &SaveStackWorker::saveToFile);
        connect(worker, &SaveStackWorker::finished, thread, &QThread::quit);
        connect(worker, &SaveStackWorker::finished, worker, &SaveStackWorker::deleteLater);
        connect(worker, &SaveStackWorker::finished, this, &SPIM::stop);
        connect(worker, &SaveStackWorker::error, this, &SPIM::onError);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);

//        orca->setNFramesInBuffer(100);  //FIXME
//        orca->startCapture();

        thread->start();
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }

    emit captureStarted();
}

void SPIM::stop()
{
    try {
        if (thread && thread->isRunning()) {
            thread->requestInterruption();
            thread = nullptr;
        }
        foreach(OrcaFlash * orca, camList) {
            orca->stop();
        }
        foreach(GalvoRamp * galvoRamp, galvoList) {
            galvoRamp->stop();
        }
        cameraTrigger->stop();
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }

    emit stopped();
}

void SPIM::setExposureTime(double expTime)
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
                if (fabs(tempDouble - lineInterval) < 0.001 || tempInt != nOfLines) {
                    QString m("Different values for line interval and number of"
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

        int nSamples = static_cast<int>(round(expTime / lineInterval + nOfLines));

        foreach(GalvoRamp * galvoRamp, galvoList) {
            galvoRamp->setCameraParams(nSamples, nOfLines, 1 / lineInterval);
        }

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
    emit error(errMsg);
    logger->error(errMsg);
    stop();
}

SPIM &spim()
{
    static auto instance = std::make_unique<SPIM>(nullptr);
    return *instance;
}
