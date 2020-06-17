#include "displayworker.h"

#include <qtlab/hw/hamamatsu/orcaflash.h>
#include <qtlab/widgets/cameradisplay.h>
#include <qtlab/widgets/cameraplot.h>

#define BUFSIZE (2048 * 2048)

DisplayWorker::DisplayWorker(OrcaFlash *camera, CameraDisplay *cd, QObject *parent)
    : QThread(parent)
{
    qRegisterMetaType<size_t>("size_t");

    mybuf = new uint16_t[BUFSIZE];

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
    delete[] mybuf;
}

void DisplayWorker::run()
{
    double *mybufDouble = new double[BUFSIZE];

    running = true;
    while (true) {
        msleep(250);
        if (!running) {
            break;
        }
        try {
            orca->copyLastFrame(mybuf, BUFSIZE * sizeof(uint16_t));
        }
        catch (std::exception) {
            continue;
        }
        for (int i = 0; i < BUFSIZE; ++i) {
            mybufDouble[i] = mybuf[i];
        }
        emit newImage(mybufDouble, BUFSIZE);
    }
    delete[] mybufDouble;
}
