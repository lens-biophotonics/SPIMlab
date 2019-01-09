#include "spimhub.h"
#include "logmanager.h"

SPIMHub* SPIMHub::inst = nullptr;

static Logger *logger = LogManager::getInstance()->getLogger("SPIMHub");


SPIMHub::SPIMHub()
{
    thread = nullptr;
    worker = nullptr;
}

SPIMHub *SPIMHub::getInstance()
{
    if (!inst) {
        inst = new SPIMHub();
    }
    return inst;
}

OrcaFlash *SPIMHub::camera()
{
    return orca;
}

void SPIMHub::setCamera(OrcaFlash *camera)
{
    orca = camera;
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
    worker->setFrameCount(40);
    worker->moveToThread(thread);

    connect(thread, SIGNAL(started()), worker, SLOT(saveToFile()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(finished()), this, SLOT(stop()));

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
