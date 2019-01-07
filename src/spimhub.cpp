#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "spimhub.h"
#include "logmanager.h"

Logger *logger = LogManager::getInstance().getLogger("SPIMHub");


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

    stopRequested = false;
    logger->info("Start acquisition");
    thread = new boost::thread(boost::bind(&SPIMHub::worker, this, 200));
}

void SPIMHub::stop()
{
    if (thread) {
        thread->interrupt();
    }
    orca->stop();
    stopped();
}

void SPIMHub::requestStop()
{
    logger->info("Stop requested");
    stopRequested = true;
}

void SPIMHub::worker(uint framecount)
{
    int fd = open("/mnt/ramdisk/output.bin", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    size_t n = 2 * 2048 * 2048;
    uint16_t *buf = static_cast<uint16_t*>(malloc(n));
    for (uint i = 0; i < framecount && !stopRequested; ++i) {
        orca->copyFrame(buf, n, i % orca->nFramesInBuffer());
        write(fd, buf, n);
    }

    logger->info("worker done");
    free(buf);
    close(fd);
    stop();
}
