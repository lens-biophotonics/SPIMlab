#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "logmanager.h"
#include "savestackworker.h"
#include "spimhub.h"

static Logger *logger = LogManager::getInstance()->getLogger("SaveStackWorker");

SaveStackWorker::SaveStackWorker(QObject *parent) : QObject(parent)
{
}

void SaveStackWorker::saveToFile()
{
    stopRequested = false;
    int fd = open("/mnt/ramdisk/output.bin", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    size_t n = 2 * 2048 * 2048;
    OrcaFlash *orca = spimHub().camera();
    const uint nFramesInBuffer = orca->nFramesInBuffer();
#ifdef WITH_HARDWARE
    void *buf;
    int32_t rowBytes;
#else
    void *buf = malloc(n);
#endif
    uint i = 0;
    while (i < frameCount) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }
        int32_t frame = static_cast<int32_t>(i % nFramesInBuffer);
#ifdef WITH_HARDWARE
        if (!orca->wait()) {
            continue;
        }
        if (!orca->lockData(&buf, &rowBytes, frame)) {
            continue;
        }
#else
        orca->copyFrame(buf, n, frame);
#endif
        write(fd, buf, n);
        i++;

#ifdef WITH_HARDWARE
        orca->unlockData();
#endif
    }

#ifndef WITH_HARDWARE
    free(buf);
#endif

    logger->info(QString("Saved %1 frames").arg(i));
    close(fd);

    emit finished();
}

void SaveStackWorker::setFrameCount(uint count)
{
    frameCount = count;
}
