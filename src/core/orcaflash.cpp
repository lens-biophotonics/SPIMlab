#ifndef WITH_HARDWARE
#include <stdio.h>
#endif

#include "orcaflash.h"
#include "logger.h"

#define FUNCNAME(x) # x
#ifdef WITH_HARDWARE
#define CALL_THROW(functionCall) \
    if (!functionCall) { \
        throw std::runtime_error( \
                  logLastError(FUNCNAME(functionCall)).toStdString()); \
    }
#else
#define CALL_THROW(func)
#endif

static Logger *dcamLogger = getLogger("DCAM");
static Logger *orcaLogger = getLogger("OrcaFlash");

using namespace DCAM;

int init_dcam()
{
    int nCamera;
#ifdef WITH_HARDWARE
    if (!dcam_init(nullptr, &nCamera)) {
        QString errMsg = "Cannot initialize dcam";
        dcamLogger->critical(errMsg);
        throw std::runtime_error(errMsg.toStdString());
    }

    if (nCamera == 0) {
        QString errMsg = "No cameras found";
        dcamLogger->critical(errMsg);
        throw std::runtime_error(errMsg.toStdString());
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

void OrcaFlash::open(int index)
{
    CALL_THROW(dcam_open(&h, index))
    orcaLogger->info(QString("Camera %1 opened").arg(index));
    _isOpen = true;
}

void OrcaFlash::close()
{
    if (!_isOpen) {
        return;
    }
    CALL_THROW(dcam_close(h))
    _isOpen = false;
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
    CALL_THROW(dcam_getlasterror(h, buf, 2048))
    return QString(buf);
#else
    return QString("Cannot get last error. Compiled in DEMO mode.");
#endif
}

double OrcaFlash::setGet(DCAM::_DCAMIDPROP property, double value)
{
#ifndef WITH_HARDWARE
    Q_UNUSED(property)
#endif
    double temp = value;
    CALL_THROW(dcam_setgetpropertyvalue(
                   h, static_cast<int32>(property), &temp))
    return temp;
}

double OrcaFlash::getPropertyValue(DCAM::_DCAMIDPROP property)
{
#ifndef WITH_HARDWARE
    Q_UNUSED(property)
#endif
    double ret = 0;
    CALL_THROW(dcam_getpropertyvalue(h, static_cast<int32>(property), &ret))
    return ret;
}

void OrcaFlash::setPropertyValue(DCAM::_DCAMIDPROP property, double value)
{
    CALL_THROW(dcam_setpropertyvalue(h, static_cast<int32>(property), value))
#ifndef WITH_HARDWARE
    Q_UNUSED(property)
    Q_UNUSED(value)
#endif
}

double OrcaFlash::getLineInterval()
{
#ifdef WITH_HARDWARE
    return getPropertyValue(DCAM_IDPROP_INTERNAL_LINEINTERVAL);
#else
    return 0.01;
#endif
}

void OrcaFlash::setOutputTrigger(OrcaFlash::ORCA_OUTPUT_TRIGGER_KIND kind,
                                 OrcaFlash::ORCA_OUTPUT_TRIGGER_SOURCE source)
{
    setPropertyValue(DCAM_IDPROP_OUTPUTTRIGGER_KIND, kind);
    setPropertyValue(DCAM_IDPROP_OUTPUTTRIGGER_SOURCE, source);
}

void OrcaFlash::setSensorMode(OrcaFlash::ORCA_SENSOR_MODE mode)
{
    setPropertyValue(DCAM_IDPROP_SENSORMODE, mode);
}

int OrcaFlash::nOfLines()
{
    return 2048;
}

QString OrcaFlash::logLastError(QString label)
{
    if (!label.isEmpty())
        label.append(" ");
    QString errMsg = getLastError().prepend(label);
    orcaLogger->error(errMsg);
    return errMsg;
}

void OrcaFlash::startCapture()
{
    CALL_THROW(dcam_freeframe(h))
    CALL_THROW(dcam_precapture(h, DCAM_CAPTUREMODE_SEQUENCE))
    CALL_THROW(dcam_allocframe(h, _nFramesInBuffer))
    CALL_THROW(dcam_capture(h))
}

void OrcaFlash::stop()
{
    CALL_THROW(dcam_idle(h))
    CALL_THROW(dcam_freeframe(h))
}

void OrcaFlash::copyFrame(void *buf, size_t n, int32_t frame)
{
#ifdef WITH_HARDWARE
    void *top;
    int32 rowbytes;

    wait();
    lockData(&top, &rowbytes, frame);
    memcpy(buf, top, n);
    unlockData();
#else
    Q_UNUSED(frame)
    FILE * f = fopen("/dev/urandom", "r");
    fread(buf, 1, n, f);
    fclose(f);
#endif
}

void OrcaFlash::copyLastFrame(void *buf, size_t n)
{
    copyFrame(buf, n, -1);
}

void OrcaFlash::wait(_DWORD timeout, DCAMWAIT_EVENT event)
{
    _DWORD dwEvent = event;
    CALL_THROW(dcam_wait(h, &dwEvent, timeout, nullptr))
#ifndef WITH_HARDWARE
    Q_UNUSED(timeout)
    Q_UNUSED(event)
#endif
}

void OrcaFlash::lockData(void **pTop, int32_t *pRowbytes, int32_t frame)
{
    mutex.lock();
    CALL_THROW(dcam_lockdata(h, pTop, pRowbytes, frame))
#ifndef WITH_HARDWARE
    Q_UNUSED(pTop)
    Q_UNUSED(pRowbytes)
    Q_UNUSED(frame)
#endif
}

void OrcaFlash::unlockData()
{
    mutex.unlock();
    CALL_THROW(dcam_unlockdata(h))
}

double OrcaFlash::getExposureTime()
{
    double sec = 0.0;
    CALL_THROW(dcam_getexposuretime(h, &sec));
#ifndef WITH_HARDWARE
    sec = 0.1;
#endif
    exposureTime = sec;
    return exposureTime;
}

double OrcaFlash::setGetExposureTime(double sec)
{
#ifndef WITH_HARDWARE
    Q_UNUSED(sec)
#endif
    CALL_THROW(dcam_setexposuretime(h, sec))
    getExposureTime();
    return exposureTime;
}

OrcaFlash::ORCA_TRIGGER_MODE OrcaFlash::getTriggerMode()
{
    int32 mode = 0;
    CALL_THROW(dcam_gettriggermode(h, &mode))
    triggerMode = static_cast<ORCA_TRIGGER_MODE>(mode);
    return triggerMode;
}

OrcaFlash::ORCA_TRIGGER_MODE OrcaFlash::setGetTriggerMode(ORCA_TRIGGER_MODE mode)
{
    CALL_THROW(dcam_settriggermode(h, mode))
    getTriggerMode();
    return triggerMode;
}

OrcaFlash::ORCA_TRIGGER_POLARITY OrcaFlash::getTriggerPolarity()
{
    int32 polarity = 0;
    CALL_THROW(dcam_gettriggerpolarity(h, &polarity))

    triggerPolarity = static_cast<ORCA_TRIGGER_POLARITY>(polarity);
    return triggerPolarity;
}

OrcaFlash::ORCA_TRIGGER_POLARITY OrcaFlash::setGetTriggerPolarity(OrcaFlash::ORCA_TRIGGER_POLARITY polarity)
{
    CALL_THROW(dcam_settriggerpolarity(h, polarity))
    getTriggerPolarity();
    return triggerPolarity;
}

OrcaFlash::ORCA_STATUS OrcaFlash::getStatus()
{
    _DWORD status = 0;
    CALL_THROW(dcam_getstatus(h, &status))
    return static_cast<ORCA_STATUS>(status);
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
