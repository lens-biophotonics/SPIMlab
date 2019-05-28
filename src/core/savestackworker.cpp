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

using namespace DCAM;

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
    const int32_t nFramesInBuffer = orca->nFramesInBuffer();
    int i = 0;
    void *buf;
    int n = 2 * 2048 * 2048;
#ifdef WITH_HARDWARE
#else
    buf = malloc(n);
#endif

    stopped = false;

    connect(orca, &OrcaFlash::stopped, this, [ = ] () {
        stopped = true;
    });

    int fd = open(rawFileName().toLatin1(), O_WRONLY);

    while (!stopped && i < frameCount) {
        int32 mask = DCAMWAIT_CAPEVENT_FRAMEREADY | DCAMWAIT_CAPEVENT_STOPPED;
        int32 event;

#ifdef WITH_HARDWARE
        try {
            event = orca->wait(1000, mask);
        }
        catch (std::runtime_error) {
        }
#endif

        int32_t frame = i % nFramesInBuffer;
        int32_t frameStamp = -1;

        switch (event) {
        case DCAMWAIT_CAPEVENT_FRAMEREADY:
            try {
#ifdef WITH_HARDWARE
                orca->lockFrame(frame, &buf, &frameStamp);
#else
                orca->copyFrame(buf, n, frame);
                frameStamp = frame;
#endif
            }
            catch (std::runtime_error) {
                continue;
            }
            write(fd, buf, n);
            i++;
            break;
        case DCAMERR_TIMEOUT:
            break;
        default:
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
    out << "ObjectType = Image" << endl;
    out << "NDims = 3" << endl;
    out << "BinaryData = True" << endl;
    out << "BinaryDataByteOrderMSB = False" << endl;
    out << "DimSize = 2048 2048 " << i << endl;
    out << "ElementType = MET_USHORT" << endl;
    out << "ElementDataFile = " << fi.fileName() << endl;
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
