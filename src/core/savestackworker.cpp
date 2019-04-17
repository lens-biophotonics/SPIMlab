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

SaveStackWorker::~SaveStackWorker()
{
    logger->info("Deleting SaveStackWorker");
}

void SaveStackWorker::run()
{
    QFileInfo fi = QFileInfo(outputFileName + ".raw");
    stopRequested = false;
    int fd = open(fi.filePath().toStdString().c_str(),
                  O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        emit error(QString("Cannot open output file %1").arg(outputFileName));
        return;
    }
    size_t n = 2 * 2048 * 2048;
    int ret = posix_fallocate(fd, 0, n * frameCount);
    logger->info(QString("fallocate: %1").arg(ret));
    const int32_t nFramesInBuffer = orca->nFramesInBuffer();
    int i = 0;
    void *buf;
#ifdef WITH_HARDWARE
#else
    buf = malloc(n);
#endif

    connect(orca, &OrcaFlash::stopped, this, [ = ] () {
        stopRequested = true;
    });

    while (i < frameCount) {
        int32_t frame = i % nFramesInBuffer;
        int32_t frameStamp = -1;
        while (!stopRequested) {
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
        if (stopRequested) {
            break;
        }
        if (i == frameStamp) {
//            logger->info(QString("OK i=%1 frame=%2 framestamp=%3").arg(i).arg(frame).arg(frameStamp));
            write(fd, buf, n);
            i++;
        }
        else if (i > frameStamp) {  // try again
//            logger->warning(QString("OK i=%1 frame=%2 framestamp=%3").arg(i).arg(frame).arg(frameStamp));
            usleep(5000);
        }
        else {  // lost frame
//            logger->error(QString("KO i=%1 frame=%2 framestamp=%3").arg(i).arg(frame).arg(frameStamp));
            logger->error("Lost frame");
            break;
        }
    }
#ifdef WITH_HARDWARE
#else
    free(buf);
#endif

    orca->cap_stop();
    logger->info(QString("Saved %1 frames").arg(i));
    close(fd);

    const QString mhdFileName = outputFileName + ".mhd";
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
