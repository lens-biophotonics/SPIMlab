#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QFileInfo>
#include <QVector>

#include <qtlab/core/logger.h>
#include <qtlab/hw/hamamatsu/orcaflash.h>

#include "savestackworker.h"
#include "spim.h"

static Logger *logger = getLogger("SaveStackWorker");

using namespace DCAM;

void performBinning(uint binning, uint16_t *buf, uint16_t *obuf)
{
    size_t width = 2048 / binning;
    size_t height = 2048 / binning;
    uint binningSq = binning * binning;

    for (size_t oj = 0; oj < height; ++oj) {
        for (size_t oi = 0; oi < width; ++oi) {
            size_t jFrom = oj * binning;
            size_t iFrom = oi * binning;
            double temp = 0;
            for (uint j = 0; j < binning; ++j) {
                for (uint i = 0; i < binning; ++i) {
                    temp += buf[(jFrom + j) * 2048 + iFrom + i];
                }
            }
            *(obuf++) = temp / binningSq;
        }
    }
}

SaveStackWorker::SaveStackWorker(OrcaFlash *orca, QObject *parent)
    : QObject(parent), orca(orca)
{
    frameCount = readFrames = 0;
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

void SaveStackWorker::start()
{
    void *buf;
    size_t width = 2048;
    size_t height = 2048;
    int n = 2 * width * height;
    int binned_n = n / binning / binning;

    readFrames = 0;
    triggerCompleted = false;
    stopped = false;

    logger->info(QString("Total number of frames to acquire: %1").arg(frameCount));

#ifdef WITH_HARDWARE
    const int32_t nFramesInBuffer = orca->nFramesInBuffer();
    QVector<qint64> timeStamps(frameCount, 0);
#else
    buf = malloc(n);
#endif

    uint16_t *binnedBuf = nullptr;
    if (binning > 1) {
        binnedBuf = new uint16_t[binned_n];
    }

    int fd = open(rawFileName().toLatin1(), O_WRONLY | O_CREAT | O_TRUNC, 0666);

    while (!stopped && readFrames < frameCount) {
#ifdef WITH_HARDWARE
        int32_t frame = readFrames % nFramesInBuffer;
        int32_t frameStamp = -1;

        DCAM_TIMESTAMP timeStamp;

        quint32 mask = DCAMWAIT_CAPEVENT_FRAMEREADY | DCAMWAIT_CAPEVENT_STOPPED;
        quint32 event = DCAMWAIT_CAPEVENT_FRAMEREADY;

        if (!triggerCompleted) {
            try {
                event = orca->wait(1000, mask);
            }
            catch (std::runtime_error e) {
                continue;
            }
        }

        if (stopped) {
            break;
        }

        switch (event) {
        case DCAMWAIT_CAPEVENT_FRAMEREADY:
            try {
                orca->lockFrame(frame, &buf, &frameStamp, &timeStamp);
            }
            catch (std::runtime_error) {
                continue;
            }

            timeStamps[readFrames] = timeStamp.sec * 1e6 + timeStamp.microsec;
            if (readFrames != 0) {
                double delta = double(timeStamps[readFrames]) - double(timeStamps[readFrames - 1]);
                if (abs(delta) > timeout) {
                    logger->warning(timeoutString(delta, readFrames));
                }
                if (abs(delta) > 10e6) {  // greater than 10 seconds
                    logger->critical(timeoutString(delta, readFrames));
                    stop();
                    break;
                }
            }
            if (frameStamp != readFrames) {
                QString msg("Camera %1: Lost frame #%2 (current framestamp = %3)");
                msg = msg.arg(orca->getCameraIndex()).arg(readFrames).arg(frameStamp);
                logger->critical(msg);
                stop();
                break;
            }
            break;

        case DCAMERR_TIMEOUT:
        default:
            logger->warning(QString("Camera %1 timeout").arg(orca->getCameraIndex()));
            continue;
        }
#else
        orca->copyLastFrame(buf, n);
#endif

        if (stopped) {
            break;
        }

        if (binning > 1) {
            performBinning(binning, static_cast<uint16_t *>(buf), binnedBuf);
        }
        ssize_t written =  write(fd, binning > 1 ? binnedBuf : buf, binned_n);
        if (written != binned_n) {
            logger->critical(QString("Camera %1: written %2/%3 bytes")
                             .arg(orca->getCameraIndex())
                             .arg(written)
                             .arg(binned_n));
        }
        readFrames++;
    }

#ifdef WITH_HARDWARE
#else
    free(buf);
#endif
    close(fd);

    if (binnedBuf != nullptr) {
        delete [] binnedBuf;
    }

    emit captureCompleted(readFrames == frameCount);
    QString msg = QString("Camera %1: Saved %2/%3 frames")
                  .arg(orca->getCameraIndex()).arg(readFrames).arg(frameCount);
    if (readFrames != frameCount) {
        logger->warning(msg);
    } else {
        logger->info(msg);
    }

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
    out << "DimSize = " << width / binning << " " << height / binning << " " << readFrames << endl;
    out << "ElementType = MET_USHORT" << endl;
    out << "ElementDataFile = " << fi.fileName() << endl;
    outFile.close();
}

void SaveStackWorker::stop()
{
    stopped = true;
}

size_t SaveStackWorker::getReadFrames() const
{
    return readFrames;
}

int32_t SaveStackWorker::getFrameCount() const
{
    return frameCount;
}

void SaveStackWorker::setOutputPath(const QString &value)
{
    outputPath = value;
}

QString SaveStackWorker::timeoutString(double delta, int i)
{
    return QString("Camera %1: detected delta of %2 ms at frame %3 (timeout: %4 ms)")
           .arg(orca->getCameraIndex())
           .arg(delta * 1e-3)
           .arg(i + 1)
           .arg(timeout / 1e3);
}

void SaveStackWorker::setBinning(const uint &value)
{
    binning = value;
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

void SaveStackWorker::signalTriggerCompletion()
{
    triggerCompleted = true;
}

double SaveStackWorker::getTimeout() const
{
    return timeout;
}

void SaveStackWorker::setTimeout(double value)
{
    timeout = value;
}
