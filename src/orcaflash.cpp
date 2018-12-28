#include "orcaflash.h"
#include "logmanager.h"

#ifndef WITH_HARDWARE
#include <stdio.h>
#endif

static Logger *dcamLogger = LogManager::getInstance().getLogger("DCAM");
static Logger *orcaLogger = LogManager::getInstance().getLogger("OrcaFlash");

using namespace DCAM;

int init_dcam()
{
    int nCamera;
#ifdef WITH_HARDWARE
    if (!dcam_init(nullptr, &nCamera)) {
        dcamLogger->critical("Cannot initialize dcam");
        return 0;
    }

    if (nCamera == 0) {
        dcamLogger->critical("No cameras found");
    }
#else
    nCamera = 8;
#endif
    dcamLogger->info(QString("Found %1 cameras").arg(nCamera));
    return nCamera;
}

bool uninit_dcam()
{
#ifdef WITH_HARDWARE
    return dcam_uninit();
#else
    return true;
#endif
}

OrcaFlash::OrcaFlash(QObject *parent) : QObject(parent)
{
}

OrcaFlash::~OrcaFlash()
{
    close();
}

bool OrcaFlash::open(int index)
{
#ifdef WITH_HARDWARE
    if (!dcam_open(&h, index)) {
        orcaLogger->critical(QString("Cannot open camera %1").arg(index));
        return false;
    }
#else
    Q_UNUSED(index)
#endif
    orcaLogger->info(QString("Camera %1 opened").arg(index));
    return true;
}

bool OrcaFlash::close()
{
#ifdef WITH_HARDWARE
    if (!dcam_close(h)) {
        logLastError("close");
        return false;
    }
#endif
    return true;
}

QString OrcaFlash::getLastError()
{
#ifdef WITH_HARDWARE
    char buf[2048];
    dcam_getlasterror(h, buf, 2048);
    return QString(buf);
#else
    return QString("Cannot get last error. Compiled in DEMO mode.");
#endif
}

bool OrcaFlash::setGet(DCAM::_DCAMIDPROP property, double value, double *get)
{
#ifdef WITH_HARDWARE
    if (!dcam_setgetpropertyvalue(h, static_cast<int32>(property), &value)) {
        logLastError("setgetvalue");
        return false;
    }
    if (get != nullptr) {
        *get = value;
    }
#else
    Q_UNUSED(property)
    Q_UNUSED(value)
    Q_UNUSED(get)
#endif
    return true;
}

void OrcaFlash::logLastError(QString label)
{
    if (!label.isEmpty())
        label.append(" ");
    orcaLogger->error(getLastError().prepend(label));
}

bool OrcaFlash::startCapture(int32_t framecount)
{
#ifdef WITH_HARDWARE
    dcam_freeframe(h);
    if (!dcam_precapture(h, DCAM_CAPTUREMODE_SEQUENCE)) {
        logLastError("precapture");
        return false;
    }

    if (!dcam_allocframe(h, framecount)) {
        logLastError("allocframe");
        return false;
    }

    if (!dcam_capture(h)) {
        logLastError("capture");
        return false;
    }
#else
    Q_UNUSED(framecount)
#endif
    return true;
}

bool OrcaFlash::stop()
{
#ifdef WITH_HARDWARE
    if (!dcam_idle(h)) {
        logLastError("idle");
        return false;
    }

    if (!dcam_freeframe(h)) {
        logLastError("freeframe");
        return false;
    }
#endif
    return true;
}

bool OrcaFlash::copyFrame(void *buf, size_t n, int32_t frame)
{
#ifdef WITH_HARDWARE
    void *top;
    int32 rowbytes;

    _DWORD dwEvent = DCAMCAP_EVENT_FRAMEREADY;
    if (!dcam_wait(h, &dwEvent, 1000, nullptr)) {
        logLastError("wait");
        return false;
    }

    if (!dcam_lockdata(h, &top, &rowbytes, frame))
    {
        logLastError("lockdata");
        return false;
    }

    memcpy(buf, top, n * 2);

    if (!dcam_unlockdata(h))
    {
        logLastError("unlockdata");
        return false;
    }
#else
    Q_UNUSED(frame)
    FILE * f = fopen("/dev/urandom", "r");
    fread(buf, 2, n, f);
    fclose(f);
#endif
    return true;
}

bool OrcaFlash::copyLastFrame(void *buf, size_t n)
{
    return copyFrame(buf, n, -1);
}

double OrcaFlash::getExposureTime()
{
#ifdef WITH_HARDWARE
    double sec;
    if (!dcam_getexposuretime(h, &sec))
    {
        logLastError("getexposuretime");
        return -1;
    }
    orcaLogger->info(QString("exposure time: %1").arg(sec));
    return sec;
#else
    return exposureTime;
#endif
}

bool OrcaFlash::setExposureTime(double sec)
{
#ifdef WITH_HARDWARE
    if (!dcam_setexposuretime(h, sec))
    {
        logLastError("setexposuretime");
        return false;
    }

    exposureTime = getExposureTime();
#else
    exposureTime = sec;
#endif
    return true;
}

OrcaFlash::ORCA_TRIGGER_MODE OrcaFlash::getTriggerMode()
{
#ifdef WITH_HARDWARE
    int32 mode;
    if (!dcam_gettriggermode(h, &mode))
    {
        logLastError("gettriggermode");
        return triggerMode;
    }

    return static_cast<ORCA_TRIGGER_MODE>(mode);
#else
    return triggerMode;
#endif
}

bool OrcaFlash::setTriggerMode(ORCA_TRIGGER_MODE mode)
{
#ifdef WITH_HARDWARE
    if (!dcam_settriggermode(h, mode))
    {
        logLastError("settriggermode");
        return false;
    }

    triggerMode = getTriggerMode();
#else
    triggerMode = mode;
#endif
    return true;
}

OrcaFlash::ORCA_TRIGGER_POLARITY OrcaFlash::getTriggerPolarity()
{
#ifdef WITH_HARDWARE
    int32 polarity;
    if (!dcam_gettriggerpolarity(h, &polarity))
    {
        logLastError("gettriggerpolarity");
        return triggerPolarity;
    }

    return static_cast<ORCA_TRIGGER_POLARITY>(polarity);
#else
    return triggerPolarity;
#endif
}

bool OrcaFlash::setTriggerPolarity(OrcaFlash::ORCA_TRIGGER_POLARITY polarity)
{
#ifdef WITH_HARDWARE
    if (!dcam_settriggerpolarity(h, polarity))
    {
        logLastError("settriggerpolarity");
        return false;
    }

    triggerPolarity = getTriggerPolarity();
#else
    triggerPolarity = polarity;
#endif
    return true;
}

OrcaFlash::ORCA_STATUS OrcaFlash::getStatus()
{
#ifdef WITH_HARDWARE
    _DWORD status;
    if (!dcam_getstatus(h, &status))
    {
        logLastError("getstatus");
        return STATUS_ERROR;
    }

    return static_cast<ORCA_STATUS>(status);
#else
    return STATUS_READY;
#endif
}
