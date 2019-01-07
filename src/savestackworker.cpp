#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "logmanager.h"
#include "savestackworker.h"
#include "spimhub.h"

static Logger *logger = LogManager::getInstance().getLogger("SaveStackWorker");

SaveStackWorker::SaveStackWorker(QObject *parent) : QObject(parent)
{
}

void SaveStackWorker::saveToFile()
{
    stopRequested = false;
    int fd = open("/mnt/ramdisk/output.bin", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    size_t n = 2 * 2048 * 2048;
    OrcaFlash *orca = SPIMHub::getInstance()->camera();
    const uint nFramesInBuffer = orca->nFramesInBuffer();
#ifdef WITH_HARDWARE
    void *buf;
    int32_t rowBytes;
#else
    void *buf = malloc(n);
#endif
    for (uint i = 0; i < frameCount; ++i) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }
        int32_t frame = static_cast<int32_t>(i % nFramesInBuffer);
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

    emit finished();
}

void SaveStackWorker::setFrameCount(uint count)
{
    frameCount = count;
}
