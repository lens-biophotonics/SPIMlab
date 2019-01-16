#include <memory>

#include "spimhub.h"
#include "logger.h"

static Logger *logger = getLogger("SPIMHub");


SPIMHub::SPIMHub()
{
    thread = nullptr;
    worker = nullptr;
}

SPIMHub::~SPIMHub()
{
    uninit_dcam();
}

void SPIMHub::initialize()
{
    init_dcam();
    orca->open(0);

    emit initialized();
}

OrcaFlash *SPIMHub::camera()
{
    return orca;
}

void SPIMHub::setCamera(OrcaFlash *camera)
{
    orca = camera;
    orca->setParent(this);
}

void SPIMHub::startFreeRun()
{
    orca->setExposureTime(0.010);
    orca->setNFramesInBuffer(10);
    orca->startCapture();
    emit captureStarted();
}

void SPIMHub::startAcquisition()
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

    orca->setExposureTime(0.2);
    orca->setNFramesInBuffer(100);
    orca->startCapture();

    thread->start();
    emit captureStarted();
}

void SPIMHub::stop()
{
    if (thread && thread->isRunning()) {
        thread->requestInterruption();
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread = nullptr;
    }
    orca->stop();
    emit stopped();
}

SPIMHub &spimHub()
{
    static auto instance = std::make_unique<SPIMHub>();
    return *instance;
}
