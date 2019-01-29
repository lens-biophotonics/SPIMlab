#include <memory>
#include <cmath>

#include "spim.h"
#include "logger.h"

static Logger *logger = getLogger("SPIMHub");


SPIM::SPIM(QObject *parent) : QObject(parent)
{
}

SPIM::~SPIM()
{
}

void SPIM::initialize()
{
    try {
        DCAM::init_dcam();

        orca = new OrcaFlash();
        orca->open(0);
        orca->setSensorMode(OrcaFlash::SENSOR_MODE_PROGRESSIVE);
        orca->setGetTriggerMode(OrcaFlash::TRIGMODE_EDGE);
        orca->setOutputTrigger(OrcaFlash::OUTPUT_TRIGGER_KIND_PROGRAMMABLE,
                               OrcaFlash::OUTPUT_TRIGGER_SOURCE_VSYNC);

        cameraTrigger = new CameraTrigger();
        galvoRamp = new GalvoRamp();
        galvoRamp->setPhysicalChannel("Dev1/ao0");
        galvoRamp->setupWaveform(0.2, 2, 0);
        setupCameraTrigger("Dev1/ctr0", "/Dev1/PFI0");
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }

    emit initialized();
}

void SPIM::uninitialize()
{
    try {
        delete orca;
        DCAM::uninit_dcam();
        delete cameraTrigger;
        delete galvoRamp;
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }
}

OrcaFlash *SPIM::camera() const
{
    return orca;
}

void SPIM::setupCameraTrigger(
    const QString &COPhysicalChan, const QString &terminal)
{
    try {
        cameraTrigger->setPhysicalChannel(COPhysicalChan);
        cameraTrigger->setTerm(terminal);

        galvoRamp->setTriggerSource(terminal);
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }
}

void SPIM::startFreeRun()
{
    try {
        setExposureTime(0.0001);
        orca->setNFramesInBuffer(10);
        orca->startCapture();

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
        thread = new QThread();
        worker = new SaveStackWorker();
        worker->setFrameCount(100);
        worker->moveToThread(thread);

        connect(thread, &QThread::started, worker, &SaveStackWorker::saveToFile);
        connect(worker, &SaveStackWorker::finished, thread, &QThread::quit);
        connect(worker, &SaveStackWorker::finished, worker, &SaveStackWorker::deleteLater);
        connect(worker, &SaveStackWorker::finished, this, &SPIM::stop);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        orca->setNFramesInBuffer(100);
        orca->startCapture();

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
        orca->stop();
        galvoRamp->stop();
        cameraTrigger->stop();
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }

    emit stopped();
}

void SPIM::setExposureTime(double expTime)
{
    try {
        expTime = orca->setGetExposureTime(expTime);

        double lineint = orca->getLineInterval();
        int nOfLines = orca->nOfLines();

        int nSamples = static_cast<int>(round(expTime / lineint + nOfLines));

        galvoRamp->setCameraParams(nSamples, nOfLines, 1 / lineint);
        double frameRate = 1 / (expTime + (nOfLines + 10) * lineint);
        cameraTrigger->setFrequency(0.98 * frameRate);
    } catch (std::runtime_error e) {
        onError(e.what());
        return;
    }
}

void SPIM::onError(const QString &errMsg) const
{
    emit error(errMsg);
}

SPIM &spim()
{
    static auto instance = std::make_unique<SPIM>(nullptr);
    return *instance;
}
