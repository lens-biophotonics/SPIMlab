#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "spimhub.h"
#include "logmanager.h"


void worker(OrcaFlash *orca, uint framecount)
{
    static Logger *logger = LogManager::getInstance().getLogger("worker");

    int fd = open("/mnt/data/output.bin", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    size_t n = 2 * 2048 * 2048;
    uint16_t *buf = static_cast<uint16_t*>(malloc(n));
    for (uint i = 0; i < framecount; ++i) {
        orca->copyFrame(buf, n, i % orca->nFramesInBuffer());
        write(fd, buf, n);
    }

    logger->info("worker done");
    free(buf);
    close(fd);
    SPIMHub::getInstance().stop();
}

SPIMHub::SPIMHub()
{
    thread = nullptr;
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
    captureStarted();
}

void SPIMHub::startAcquisition()
{
    orca->setNFramesInBuffer(100);
    orca->startCapture();

    thread = new boost::thread(worker, orca, 200);
}

void SPIMHub::stop()
{
    if (thread) {
        thread->interrupt();
    }
    orca->stop();
    stopped();
}
