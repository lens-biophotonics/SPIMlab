#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <QFile>
#include <QTextStream>
#include <QFileInfo>

#include "logger.h"
#include "savestackworker.h"
#include "spim.h"
#include "orcaflash.h"

static Logger *logger = getLogger("SaveStackWorker");

SaveStackWorker::SaveStackWorker(OrcaFlash *orca, QObject *parent)
    : QThread(parent), orca(orca)
{
}

void SaveStackWorker::run()
{
    QFileInfo fi = QFileInfo(outputFileName);
    stopRequested = false;
    int fd = open(outputFileName.toStdString().c_str(),
                  O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        emit error(QString("Cannot open output file %1").arg(outputFileName));
        return;
    }
    size_t n = 2 * 2048 * 2048;
    const int32_t nFramesInBuffer = orca->nFramesInBuffer();
    int i = 0;
    void *buf;
#ifdef WITH_HARDWARE
#else
    buf = malloc(n);
#endif
    while (i < frameCount) {
        int32_t frame = i % nFramesInBuffer;
        int32_t frameStamp = -1;
        while (true) {
            if (QThread::currentThread()->isInterruptionRequested()
                | !orca->getCapturingState()->active()) {
                break;
            }
            try {
#ifdef WITH_HARDWARE
                orca->lockFrame(frame, &buf, &frameStamp);
#else
                orca->copyFrame(buf, n, frame);
                frameStamp = frame;
#endif
                break;
            }
            catch (OrcaFlash::OrcaBusyException) {
                usleep(5000);
                continue;
            }
        }
        if (QThread::currentThread()->isInterruptionRequested()
            | !orca->getCapturingState()->active()) {
            break;
        }
        if (i == frameStamp) {
            write(fd, buf, n);
            i++;
        }
        else if (i > frameStamp) {  // try again
            usleep(5000);
        }
        else {  // lost frame
            logger->error("Lost frame");
            break;
        }
    }
#ifdef WITH_HARDWARE
#else
    free(buf);
#endif

    orca->stop();
    logger->info(QString("Saved %1 frames").arg(i));
    close(fd);

    const QString mhdFileName = fi.baseName() + ".mhd";
    QFile outFile(mhdFileName);
    if (!outFile.open(QIODevice::WriteOnly)) {
        emit error(QString("Cannot open output file %1").arg(mhdFileName));
        return;
    };
    QTextStream out(&outFile);
    out << "ObjectType = Image\n";
    out << "NDims = 3\n";
    out << "DimSize = 2048 2048 " << i << "\n";
    out << "ElementType = MET_USHORT\n";
    out << "ElementDataFile = " << fi.fileName() << "\n";
    outFile.close();
}

void SaveStackWorker::setFrameCount(int32_t count)
{
    frameCount = count;
}

void SaveStackWorker::setOutputFileName(const QString &fname)
{
    outputFileName = fname;
}
