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
    const uint nFramesInBuffer = orca->nFramesInBuffer();
    uint i = 0;
    while (i < frameCount) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }
        int32_t frame = static_cast<int32_t>(i % nFramesInBuffer);
        int32_t frameStamp;
        void *buf;
        while (true) {
            try {
                orca->lockFrame(frame, &buf, &frameStamp);
                break;
            }
            catch (OrcaFlash::OrcaBusyException) {
                continue;
            }
        }
        if (i != frameStamp) {
            logger->warning("Missed frame");
            continue;
        }
        write(fd, buf, n);
        i++;
    }

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

void SaveStackWorker::setFrameCount(uint count)
{
    frameCount = count;
}

void SaveStackWorker::setOutputFileName(const QString &fname)
{
    outputFileName = fname;
}
