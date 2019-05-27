#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <QFile>
#include <QDir>
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

void SaveStackWorker::layOutFileOnDisk()
{
    int fd = open(rawFileName().toLatin1(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        emit error(QString("Cannot create output file %1").arg(rawFileName()));
        return;
    }
    off_t n = 2 * 2048 * 2048 * static_cast<off_t>(frameCount);
    int ret = posix_fallocate(fd, 0, n);
    close(fd);
}

void SaveStackWorker::run()
{
    stopRequested = false;

    const int32_t nFramesInBuffer = orca->nFramesInBuffer();
    int i = 0;
    void *buf;
    int n = 2 * 2048 * 2048;
#ifdef WITH_HARDWARE
#else
    buf = malloc(n);
#endif

    connect(orca, &OrcaFlash::stopped, this, [ = ] () {
        stopRequested = true;
    });

    int fd = open(rawFileName().toLatin1(), O_WRONLY);

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

    orca->cap_stop();
    logger->info(QString("Saved %1 frames").arg(i));
    close(fd);

    QFile outFile(mhdFileName());
    if (!outFile.open(QIODevice::WriteOnly)) {
        emit error(QString("Cannot open output file %1")
                   .arg(outFile.fileName()));
        return;
    };
    QFileInfo fi = QFileInfo(rawFileName());

    QTextStream out(&outFile);
    out << "ObjectType = Image\n";
    out << "NDims = 3\n";
    out << "DimSize = 2048 2048 " << i << "\n";
    out << "ElementType = MET_USHORT\n";
    out << "ElementDataFile = " << fi.fileName() << "\n";
    outFile.close();
}

void SaveStackWorker::setOutputPath(const QString &value)
{
    outputPath = value;
}

void SaveStackWorker::setFrameCount(int32_t count)
{
    frameCount = count;
}

void SaveStackWorker::setOutputFileName(const QString &fname)
{
    outputFileName = fname;
}

QString SaveStackWorker::rawFileName()
{
    return QString("%1.raw").arg(QDir(outputPath).filePath(outputFileName));
}

QString SaveStackWorker::mhdFileName()
{
    return QString("%1.mhd").arg(QDir(outputPath).filePath(outputFileName));
}
