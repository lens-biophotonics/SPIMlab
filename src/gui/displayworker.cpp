#include "displayworker.h"

#include <qtlab/hw/hamamatsu/orcaflash.h>
#include <qtlab/widgets/cameradisplay.h>
#include <qtlab/widgets/cameraplot.h>

DisplayWorker::DisplayWorker(OrcaFlash *camera, QObject *parent)
    : QThread(parent)
{
    qRegisterMetaType<size_t>("size_t");

    mybufDouble = new double[2048 * 2048];

    orca = camera;

    connect(orca, &OrcaFlash::captureStarted, this, [=]() { start(); });
    connect(orca, &OrcaFlash::stopped, this, [=]() { running = false; });
}

DisplayWorker::~DisplayWorker()
{
    delete[] mybufDouble;
}

void DisplayWorker::run()
{
    void *buf = nullptr;
    int w = orca->getImageWidth();
    int h = orca->getImageHeight();
    int bufsize = w * h;
#ifdef QTLAB_DCAM_DEMO
    quint16 *bufInt = new quint16[bufsize];
    buf = bufInt;
#endif
    running = true;

    while (true) {
        msleep(250);
        if (!running) {
#ifdef QTLAB_DCAM_DEMO
            delete[] bufInt;
#endif
            break;
        }
        try {
            orca->lockFrame(-1, &buf);
        } catch (std::exception) {
            continue;
        }
        quint16 *mybuf = static_cast<quint16 *>(buf);
        int c = 0;
        for (int i = 0; i < h; i += binning) {
            for (int j = 0; j < w; j += binning) {
                int ii = flipVertically ? h - i - 1 : i;
                mybufDouble[c++] = mybuf[ii * w + j];
            }
        }

        emit newImage(mybufDouble, bufsize / binning / binning);
    }
}

void DisplayWorker::setVerticalFlipEnabled(bool enable)
{
    flipVertically = enable;
}

void DisplayWorker::setBinning(uint value)
{
    binning = value;
}
