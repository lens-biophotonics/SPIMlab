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
        orca->setGetTriggerMode(OrcaFlash::TRIGMODE_START);

        cameraTrigger = new CameraTrigger();
        galvoRamp = new GalvoRamp();
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

OrcaFlash *SPIM::camera()
{
    return orca;
}

void SPIM::setupCameraTrigger(QString COPhysicalChan, QString terminal)
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
        orca->setGetExposureTime(0.010);
        orca->setNFramesInBuffer(10);
        orca->startCapture();
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
        thread = new QThread();
        worker = new SaveStackWorker();
        worker->setFrameCount(100);
        worker->moveToThread(thread);

        connect(thread, &QThread::started, worker, &SaveStackWorker::saveToFile);
        connect(worker, &SaveStackWorker::finished, thread, &QThread::quit);
        connect(worker, &SaveStackWorker::finished, worker, &SaveStackWorker::deleteLater);
        connect(worker, &SaveStackWorker::finished, this, &SPIM::stop);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        orca->setGetExposureTime(0.2);
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

void SPIM::onError(const QString &errMsg)
{
    emit error(errMsg);
}

SPIM &spim()
{
    static auto instance = std::make_unique<SPIM>(nullptr);
    return *instance;
}
