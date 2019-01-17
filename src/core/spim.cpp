#include <memory>
#include <cmath>

#include "spim.h"
#include "logger.h"

static Logger *logger = getLogger("SPIMHub");


SPIM::SPIM()
{
}

SPIM::~SPIM()
{
    uninit_dcam();
}

void SPIM::initialize()
{
    init_dcam();
    orca->open(0);
    orca->setGetTriggerMode(OrcaFlash::TRIGMODE_START);

    cameraTrigger = new CameraTrigger(this);
    galvoRamp = new GalvoRamp(this);

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

void SPIM::setupCameraTrigger(QString COPhysicalChan, QString terminal)
{
    cameraTrigger->setPhysicalChannel(COPhysicalChan);
    cameraTrigger->setTerm(terminal);

    galvoRamp->setTriggerSource(terminal);
}

void SPIM::startFreeRun()
{
    orca->setGetExposureTime(0.010);
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

    orca->setGetExposureTime(0.2);
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

void SPIM::setExposureTime(double expTime)
{
    expTime = orca->setGetExposureTime(expTime);

    double lineint = orca->getLineInterval();
    int nOfLines = orca->nOfLines();

    int nSamples = static_cast<int>(round(expTime / lineint + nOfLines));

    galvoRamp->setCameraParams(nSamples, nOfLines, 1 / lineint);
    double frameRate = 1 / (expTime + (nOfLines + 10) * lineint);
    cameraTrigger->setFrequency(0.98 * frameRate);
}

SPIM &spim()
{
    static auto instance = std::make_unique<SPIM>();
    return *instance;
}
