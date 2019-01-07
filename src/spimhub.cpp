#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "spimhub.h"
#include "logmanager.h"


SPIMHub* SPIMHub::inst = nullptr;

Logger *logger = LogManager::getInstance().getLogger("SPIMHub");


SPIMHub::SPIMHub()
{
    thread = nullptr;
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
#ifdef WITH_HARDWARE
    void *buf;
    int32_t rowBytes;
#else
    void *buf = malloc(n);
#endif
    for (uint i = 0; i < framecount && !stopRequested; ++i) {
        int32_t frame = static_cast<int32_t>(i % orca->nFramesInBuffer());
#ifdef WITH_HARDWARE
        orca->lockData(&buf, &rowBytes, frame);
#else
        orca->copyFrame(buf, n, frame);
#endif

        write(fd, buf, n);

#ifdef WITH_HARDWARE
        orca->unlockData();
#endif
    }

#ifndef WITH_HARDWARE
    free(buf);
#endif

    logger->info("worker done");
    close(fd);
    stop();
}
