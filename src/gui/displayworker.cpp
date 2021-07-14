#include "displayworker.h"

#include <qtlab/hw/hamamatsu/orcaflash.h>
#include <qtlab/widgets/cameradisplay.h>
#include <qtlab/widgets/cameraplot.h>

#define BUFSIZE (2048 * 2048)

DisplayWorker::DisplayWorker(OrcaFlash *camera, CameraDisplay *cd, QObject *parent)
    : QThread(parent)
{
    qRegisterMetaType<size_t>("size_t");

    mybufDouble = new double[BUFSIZE];

    orca = camera;
    this->cd = cd;

    connect(orca, &OrcaFlash::captureStarted, this, [ = ](){
        start();
    });
    connect(orca, &OrcaFlash::stopped, this, [ = ](){
        running = false;
    });
}

DisplayWorker::~DisplayWorker()
{
    delete[] mybufDouble;
}

void DisplayWorker::run()
{
    void *buf = nullptr;
#ifdef QTLAB_DCAM_DEMO
    quint16 *bufInt = new quint16[BUFSIZE];
    buf = bufInt;
#endif
    running = true;
    while (true) {
        msleep(250);
        if (!running) {
#ifdef QTLAB_DCAM_DEMO
            delete [] bufInt;
#endif
            break;
        }
        try {
            orca->lockFrame(-1, &buf);
        }
        catch (std::exception) {
            continue;
        }
        quint16 *mybuf = static_cast<quint16 *>(buf);
        for (int i = 0; i < BUFSIZE; ++i) {
            mybufDouble[i] = mybuf[i];
        }
        emit newImage(mybufDouble, BUFSIZE);
    }
}
