#include <memory>

#include "spim.h"
#include "logger.h"

static Logger *logger = getLogger("SPIMHub");


SPIM::SPIM()
{
    thread = nullptr;
    worker = nullptr;
}

SPIM::~SPIM()
{
    uninit_dcam();
}

void SPIM::initialize()
{
    init_dcam();
    orca->open(0);

    emit initialized();
}

OrcaFlash *SPIM::camera()
{
    return orca;
}

void SPIM::setCamera(OrcaFlash *camera)
{
    orca = camera;
    orca->setParent(this);
}

void SPIM::startFreeRun()
{
    orca->setExposureTime(0.010);
    orca->setNFramesInBuffer(10);
    orca->startCapture();
    emit captureStarted();
}

void SPIM::startAcquisition()
{
    logger->info("Start acquisition");

    thread = new QThread();
    worker = new SaveStackWorker();
    worker->setFrameCount(100);
    worker->moveToThread(thread);

    connect(thread, SIGNAL(started()), worker, SLOT(saveToFile()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(finished()), this, SLOT(stop()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    orca->setExposureTime(0.2);
    orca->setNFramesInBuffer(100);
    orca->startCapture();

    thread->start();
    emit captureStarted();
}

void SPIM::stop()
{
    if (thread && thread->isRunning()) {
        thread->requestInterruption();
        thread = nullptr;
    }
    orca->stop();
    emit stopped();
}

SPIM &spim()
{
    static auto instance = std::make_unique<SPIM>();
    return *instance;
}
