#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QFileInfo>

#include <qtlab/core/logger.h>
#include <qtlab/hw/hamamatsu/orcaflash.h>

#include "savestackworker.h"
#include "spim.h"

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
    uint64_t timeStamps[frameCount] = {0};

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
#ifdef WITH_HARDWARE
        int32_t frame = i % nFramesInBuffer;
        int32_t frameStamp = -1;

        DCAM_TIMESTAMP timeStamp;

        int32 mask = DCAMWAIT_CAPEVENT_FRAMEREADY | DCAMWAIT_CAPEVENT_STOPPED;
        int32 event;
        try {
            event = orca->wait(1000, mask);
        }
        catch (std::runtime_error) {
        }

        switch (event) {
        case DCAMWAIT_CAPEVENT_FRAMEREADY:
            try {
                orca->lockFrame(frame, &buf, &frameStamp, &timeStamp);
                timeStamps[i] = timeStamp.sec * 1e6 + timeStamp.microsec;
                if (i != 0) {
                    double delta = double(timeStamps[i]) - double(timeStamps[i - 1]);
                    if (abs(delta) > timeout) {
                        logger->critical(QString("Camera %1 timeout by %2 ms at frame %3")
                                         .arg(orca->getCameraIndex()).arg(delta * 1e-3).arg(i + 1));
                    }
                    else if (abs(delta) > timeout * 0.75 || abs(delta) < timeout * 0.25) {
                        logger->warning(QString("Camera %1 timeout by %2 ms at frame %3")
                                        .arg(orca->getCameraIndex()).arg(delta * 1e-3).arg(i + 1));
                    }
                }
            }
            catch (std::runtime_error) {
                continue;
            }
            write(fd, buf, n);
            i++;
            break;
        case DCAMERR_TIMEOUT:
            logger->warning(QString("Camera %1 timeout").arg(orca->getCameraIndex()));
            break;
        default:
            break;
        }
#else
        orca->copyLastFrame(buf, n);
        write(fd, buf, n);
        i++;
#endif
    }

#ifdef WITH_HARDWARE
#else
    free(buf);
#endif

    emit captureCompleted();
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

double SaveStackWorker::getTimeout() const
{
    return timeout;
}

void SaveStackWorker::setTimeout(double value)
{
    timeout = value;
}
