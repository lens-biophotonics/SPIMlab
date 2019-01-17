#include "orcaflash.h"
#include "logger.h"

#ifndef WITH_HARDWARE
#include <stdio.h>
#endif

static Logger *dcamLogger = getLogger("DCAM");
static Logger *orcaLogger = getLogger("OrcaFlash");

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
    setNFramesInBuffer(10);
    _isOpen = false;
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
    _isOpen = true;
    return true;
}

bool OrcaFlash::close()
{
    if (!_isOpen) {
        return true;
    }
#ifdef WITH_HARDWARE
    if (!dcam_close(h)) {
        logLastError("close");
        return false;
    }
#endif
    _isOpen = false;
    return true;
}

void OrcaFlash::setNFramesInBuffer(uint count)
{
    _nFramesInBuffer = count;
}

uint OrcaFlash::nFramesInBuffer()
{
    return _nFramesInBuffer;
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

double OrcaFlash::getPropertyValue(DCAM::_DCAMIDPROP property)
{
    double ret = 0;
#ifdef WITH_HARDWARE
    if (!dcam_getpropertyvalue(h, static_cast<int32>(property), &ret)) {
        logLastError("dcam_getpropertyvalue");
    };
#else
    Q_UNUSED(property)
#endif
    return ret;
}

double OrcaFlash::getFrameRate()
{
    return getPropertyValue(DCAM_IDPROP_INTERNALFRAMERATE);
}

double OrcaFlash::getLineInterval()
{
    return getPropertyValue(DCAM_IDPROP_INTERNAL_LINEINTERVAL);
}

int OrcaFlash::nOfLines()
{
    return 2048;
}

void OrcaFlash::logLastError(QString label)
{
    if (!label.isEmpty())
        label.append(" ");
    orcaLogger->error(getLastError().prepend(label));
}

bool OrcaFlash::startCapture()
{
#ifdef WITH_HARDWARE
    dcam_freeframe(h);
    if (!dcam_precapture(h, DCAM_CAPTUREMODE_SEQUENCE)) {
        logLastError("precapture");
        return false;
    }

    if (!dcam_allocframe(h, _nFramesInBuffer)) {
        logLastError("allocframe");
        return false;
    }

    if (!dcam_capture(h)) {
        logLastError("capture");
        return false;
    }
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

    if (!wait()) {
        return false;
    }

    if (!lockData(&top, &rowbytes, frame)) {
        return false;
    }

    memcpy(buf, top, n);

    if (!unlockData()) {
        return false;
    }
#else
    Q_UNUSED(frame)
    FILE * f = fopen("/dev/urandom", "r");
    fread(buf, 1, n, f);
    fclose(f);
#endif
    return true;
}

bool OrcaFlash::copyLastFrame(void *buf, size_t n)
{
    return copyFrame(buf, n, -1);
}

bool OrcaFlash::wait(_DWORD timeout, DCAMWAIT_EVENT event)
{
#ifdef WITH_HARDWARE
    _DWORD dwEvent = event;
    if (!dcam_wait(h, &dwEvent, timeout, nullptr)) {
        logLastError("wait");
        return false;
    }
#else
    Q_UNUSED(timeout)
    Q_UNUSED(event)
#endif
    return true;
}

bool OrcaFlash::lockData(void **pTop, int32_t *pRowbytes, int32_t frame)
{
    mutex.lock();
#ifdef WITH_HARDWARE
    if (!dcam_lockdata(h, pTop, pRowbytes, frame))
    {
        logLastError("lockdata");
        return false;
    }
#else
    Q_UNUSED(pTop)
    Q_UNUSED(pRowbytes)
    Q_UNUSED(frame)
#endif
    return true;
}

bool OrcaFlash::unlockData()
{
    mutex.unlock();
#ifdef WITH_HARDWARE
    if (!dcam_unlockdata(h))
    {
        logLastError("unlockdata");
        return false;
    }
#endif
    return true;
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

double OrcaFlash::setGetExposureTime(double sec)
{
#ifdef WITH_HARDWARE
    if (!dcam_setexposuretime(h, sec))
    {
        logLastError("setexposuretime");
        return exposureTime;
    }

    exposureTime = getExposureTime();
#else
    exposureTime = sec;
#endif
    return exposureTime;
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

QString OrcaFlash::getStatusString()
{
    return statusString(getStatus());
}

QString OrcaFlash::statusString(OrcaFlash::ORCA_STATUS status)
{
    switch (status) {
    case OrcaFlash::STATUS_BUSY:
        return "busy";
    case OrcaFlash::STATUS_ERROR:
        return "error";
    case OrcaFlash::STATUS_READY:
        return "ready";
    case OrcaFlash::STATUS_STABLE:
        return "stable";
    case OrcaFlash::STATUS_UNSTABLE:
        return "unstable";
    }
}

void OrcaFlash::logStatusString()
{
    orcaLogger->info(QString("Camera status: %1").arg(getStatusString()));
}
