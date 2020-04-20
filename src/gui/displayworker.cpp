#include "displayworker.h"

#include <qtlab/hw/hamamatsu/orcaflash.h>

DisplayWorker::DisplayWorker(OrcaFlash *camera, double *data, QObject *parent)
    : QThread(parent)
{
    mybuf = new uint16_t[2048 * 2048];

    orca = camera;
    buf = data;

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
    running = true;
    while (true) {
        msleep(250);
        if (!running) {
            break;
        }
        try {
            orca->copyLastFrame(mybuf, 2048 * 2048 * sizeof(uint16_t));
        }
        catch (std::exception) {
            continue;
        }
        for (int i = 0; i < 2048 * 2048; ++i) {
            buf[i] = mybuf[i];
        }
        emit newImage();
    }
}
