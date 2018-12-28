#include "spimhub.h"

SPIMHub::SPIMHub()
{
}

OrcaFlash *SPIMHub::camera()
{
    return orca;
}

void SPIMHub::setCamera(OrcaFlash *camera)
{
    orca = camera;
}

void SPIMHub::startFreeRun()
{
    orca->setExposureTime(0.010);
    orca->startCapture();
    captureStarted();
}

void SPIMHub::stop()
{
    orca->stop();
    stopped();
}
