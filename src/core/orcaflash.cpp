#include <QStateMachine>

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

static Logger *logger = getLogger("OrcaFlash");

using namespace DCAM;


#ifdef WITH_HARDWARE
#else
#define dcambuf_lockframe(...) DCAMERR_SUCCESS
#define dcambuf_alloc(...) DCAMERR_SUCCESS
#define dcambuf_release(...) DCAMERR_SUCCESS
#define dcamwait_start(...) DCAMERR_SUCCESS
#define dcamwait_open(...) DCAMERR_SUCCESS
#define dcamwait_close(...) DCAMERR_SUCCESS
#define dcamcap_start(...) DCAMERR_SUCCESS
#define dcamcap_stop(...) DCAMERR_SUCCESS
#define dcamdev_open(...) DCAMERR_SUCCESS
#define dcamprop_setvalue(...) DCAMERR_SUCCESS
#define dcamprop_getvalue(...) DCAMERR_SUCCESS
#define dcamprop_setgetvalue(...) DCAMERR_SUCCESS
#endif

OrcaFlash::OrcaFlash(QObject *parent) : QObject(parent)
{
    _isOpen = false;
    setNFramesInBuffer(10);
    setupStateMachine();
}

OrcaFlash::~OrcaFlash()
{
    close();
}

void OrcaFlash::setupStateMachine()
{
    idleState = new QState();
    closedState = new QState();
    capturingState = new QState();

    idleState->addTransition(this, &OrcaFlash::closed, closedState);
    idleState->addTransition(this, &OrcaFlash::captureStarted, capturingState);

    capturingState->addTransition(this, &OrcaFlash::stopped, idleState);

    closedState->addTransition(this, &OrcaFlash::opened, idleState);

    QStateMachine *sm = new QStateMachine();

    sm->addState(idleState);
    sm->addState(closedState);
    sm->addState(capturingState);

    sm->setInitialState(closedState);
    sm->start();
}

void OrcaFlash::throw400(const DCAMERR err)
{
    switch (err) {
    case DCAMERR_SUCCESS:
        return;
    case DCAMERR_BUSY:
        throw OrcaBusyException();
    case DCAMERR_NOTREADY:
        throw OrcaNotReadyException();
    case DCAMERR_NOTSTABLE:
        throw OrcaNotStableException();
    case DCAMERR_UNSTABLE:
        throw OrcaUnstableException();
    case DCAMERR_NOTBUSY:
        throw OrcaNotBusyException();
    default:
        throw std::runtime_error(getLastError().toStdString());
    }
}

QState *OrcaFlash::getIdleState() const
{
    return idleState;
}

QState *OrcaFlash::getClosedState() const
{
    return closedState;
}

QState *OrcaFlash::getCapturingState() const
{
    return capturingState;
}

void OrcaFlash::open(const int index)
{
    if (_isOpen) {
        return;
    }
#if DCAM_VERSION == 400
    DCAMDEV_OPEN param;
    memset (&param, 0, sizeof(param));

    param.index = index;
    param.size = sizeof(param);
    throw400(dcamdev_open(&param));
    h = param.hdcam;
#else
    CALL_THROW(dcam_open(&h, index))
#endif
    logger->info(QString("Camera %1 opened").arg(index));
    _isOpen = true;
    cameraIndex = index;
    emit opened();
}

void OrcaFlash::open(const QString &idStr)
{
    open(getCameraIndex(idStr));
}

bool OrcaFlash::isOpen() const
{
    return _isOpen;
}

void OrcaFlash::close()
{
    if (!_isOpen) {
        return;
    }
    CALL_THROW(dcam_close(h))
    _isOpen = false;
    emit closed();
}

const ModelInfo *OrcaFlash::modelInfo()
{
    return getModelInfo(cameraIndex);
}

void OrcaFlash::setNFramesInBuffer(const int32_t count)
{
    _nFramesInBuffer = count;
}

int32_t OrcaFlash::nFramesInBuffer() const
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

double OrcaFlash::setGet(const _DCAMIDPROP property, const double value)
{
#ifndef WITH_HARDWARE
    Q_UNUSED(property)
#endif
    double temp = value;
#if DCAM_VERSION == 400
    throw400(dcamprop_setgetvalue(h, static_cast<int32>(property), &temp));
#else
    CALL_THROW(dcam_setgetpropertyvalue(
                   h, static_cast<int32>(property), &temp))
#endif
    return temp;
}

double OrcaFlash::getPropertyValue(const _DCAMIDPROP property)
{
#ifndef WITH_HARDWARE
    Q_UNUSED(property)
#endif
    double ret = 0;
#if DCAM_VERSION == 400
    throw400(dcamprop_getvalue(h, static_cast<int32>(property), &ret));
#else
    CALL_THROW(dcam_getpropertyvalue(h, static_cast<int32>(property), &ret))
#endif
    return ret;
}

void OrcaFlash::setPropertyValue(const _DCAMIDPROP property, const double value)
{
#if DCAM_VERSION == 400
    throw400(dcamprop_setvalue(h, static_cast<int32>(property), value));
#else
    CALL_THROW(dcam_setpropertyvalue(h, static_cast<int32>(property), value))
#endif
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

void OrcaFlash::setOutputTrigger(
    const OrcaFlash::ORCA_OUTPUT_TRIGGER_KIND kind,
    const OrcaFlash::ORCA_OUTPUT_TRIGGER_SOURCE source,
    const double period)
{
    setPropertyValue(DCAM_IDPROP_OUTPUTTRIGGER_KIND, kind);
    setPropertyValue(DCAM_IDPROP_OUTPUTTRIGGER_SOURCE, source);
    setPropertyValue(DCAM_IDPROP_OUTPUTTRIGGER_PERIOD, period);
}

void OrcaFlash::setSensorMode(const OrcaFlash::ORCA_SENSOR_MODE mode)
{
    setPropertyValue(DCAM_IDPROP_SENSORMODE, mode);
}

int OrcaFlash::nOfLines() const
{
    return 2048;
}

int32_t OrcaFlash::getImageWidth()
{
#if DCAM_VERSION == 400
    double dValue = getPropertyValue(DCAM_IDPROP_IMAGE_WIDTH);
    return static_cast<int32_t>(dValue);
#else
    DCAM_SIZE sz;
    CALL_THROW(dcam_getdatasizeex(h, &sz))

    return sz.cx;
#endif
}

int32_t OrcaFlash::getImageHeight()
{
#if DCAM_VERSION == 400
    double dValue = getPropertyValue(DCAM_IDPROP_IMAGE_HEIGHT);
    return static_cast<int32_t>(dValue);
#else
    DCAM_SIZE sz;
    CALL_THROW(dcam_getdatasizeex(h, &sz))

    return sz.cy;
#endif
}

QString OrcaFlash::logLastError(const QString label)
{
    QString errMsg = getLastError().prepend(label + " ");
    logger->error(errMsg);
    return errMsg;
}

void OrcaFlash::startCapture()
{
#if DCAM_VERSION == 400
    throw400(dcambuf_release(h));
    throw400(dcambuf_alloc(h, _nFramesInBuffer));
    throw400(dcamcap_start(h, DCAMCAP_START_SEQUENCE));
#else
    CALL_THROW(dcam_freeframe(h))
    CALL_THROW(dcam_precapture(h, DCAM_CAPTUREMODE_SEQUENCE))
    CALL_THROW(dcam_allocframe(h, _nFramesInBuffer))
    CALL_THROW(dcam_capture(h))
#endif
    emit captureStarted();
}

void OrcaFlash::stop()
{
    if (!_isOpen) {
        return;
    }
#if DCAM_VERSION == 400
    throw400(dcamcap_stop(h));
    throw400(dcambuf_release(h));
#else
    CALL_THROW(dcam_idle(h))
    CALL_THROW(dcam_freeframe(h))
#endif
    emit stopped();
}

void OrcaFlash::copyFrame(void * const buf, const size_t n, const int32_t frame)
{
#ifndef WITH_HARDWARE
    Q_UNUSED(frame)
    FILE * f = fopen("/dev/urandom", "r");
    fread(buf, 1, n, f);
    fclose(f);
    return;
#else

#if DCAM_VERSION == 400
    DCAMBUF_FRAME dcamframe;

    memset(&dcamframe, 0, sizeof(dcamframe));
    dcamframe.size = sizeof(dcamframe);
    dcamframe.iFrame = frame;

    dcamframe.buf = buf;
    dcamframe.width = getImageWidth();
    dcamframe.height = getImageHeight();
    dcamframe.rowbytes = static_cast<int32_t>(
        getPropertyValue(DCAM_IDPROP_IMAGE_ROWBYTES));
    dcamframe.left = 0;
    dcamframe.top = 0;

    throw400(dcambuf_copyframe(h, &dcamframe));
#else
    void *top;
    int32 rowbytes;

    wait();
    lockData(&top, &rowbytes, frame);
    memcpy(buf, top, n);
    unlockData();
#endif
#endif
}

void OrcaFlash::lockFrame(const int32_t frame, void **buf, int32_t *frameStamp)
{
#if DCAM_VERSION == 400
    DCAMBUF_FRAME dcamframe;

    memset(&dcamframe, 0, sizeof(dcamframe));
    dcamframe.size = sizeof(dcamframe);
    dcamframe.iFrame = frame;

    dcamframe.width = getImageWidth();
    dcamframe.height = getImageHeight();
    dcamframe.rowbytes = static_cast<int32_t>(
        getPropertyValue(DCAM_IDPROP_IMAGE_ROWBYTES));
    dcamframe.left = 0;
    dcamframe.top = 0;

    lockFrame(&dcamframe);

    *buf = dcamframe.buf;
    if (frameStamp != nullptr) {
#ifdef WITH_HARDWARE
        *frameStamp = dcamframe.framestamp;
#else
        *frameStamp = frame;
#endif
    }
#endif
}

void OrcaFlash::lockFrame(DCAMBUF_FRAME *dcambufFrame)
{
#ifndef WITH_HARDWARE
    Q_UNUSED(dcambufFrame)
#endif
    throw400(dcambuf_lockframe(h, dcambufFrame));
}

void OrcaFlash::copyLastFrame(void * const buf, const size_t n)
{
    copyFrame(buf, n, -1);
}

void OrcaFlash::wait(const _DWORD timeout, const DCAMWAIT_EVENT event)
{
    _DWORD dwEvent = event;
    CALL_THROW(dcam_wait(h, &dwEvent, timeout, nullptr))
#ifndef WITH_HARDWARE
    Q_UNUSED(dwEvent)
    Q_UNUSED(timeout)
    Q_UNUSED(event)
#endif
}

int32 OrcaFlash::wait(const int32 timeout_ms, const int32 eventMask)
{
    DCAMWAIT_OPEN dwOpen;
    dwOpen.size = sizeof(DCAMWAIT_OPEN);
    dwOpen.hdcam = h;

    throw400(dcamwait_open(&dwOpen));

    DCAMWAIT_START dwStart;
    dwStart.size = sizeof(DCAMWAIT_START);
    dwStart.eventmask = eventMask;
    dwStart.timeout = timeout_ms;

    throw400(dcamwait_start(dwOpen.hwait, &dwStart));

    throw400(dcamwait_close(dwOpen.hwait));

    return dwStart.eventhappened;
}

void OrcaFlash::lockData(void **pTop, int32_t *pRowbytes, const int32_t frame)
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

double OrcaFlash::setGetExposureTime(const double sec)
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
    return static_cast<ORCA_TRIGGER_MODE>(mode);
}

void OrcaFlash::setTriggerMode(const ORCA_TRIGGER_MODE mode)
{
    CALL_THROW(dcam_settriggermode(h, mode))
#ifndef WITH_HARDWARE
    Q_UNUSED(mode)
#endif
}

void OrcaFlash::setTriggerSource(const OrcaFlash::ORCA_TRIGGERSOURCE source)
{
    setPropertyValue(DCAM_IDPROP_TRIGGERSOURCE, source);
}

OrcaFlash::ORCA_TRIGGER_POLARITY OrcaFlash::getTriggerPolarity()
{
    int32 polarity = 0;
    CALL_THROW(dcam_gettriggerpolarity(h, &polarity))
    return static_cast<ORCA_TRIGGER_POLARITY>(polarity);
}

void OrcaFlash::setTriggerPolarity(
    const OrcaFlash::ORCA_TRIGGER_POLARITY polarity)
{
    CALL_THROW(dcam_settriggerpolarity(h, polarity))
#ifndef WITH_HARDWARE
    Q_UNUSED(polarity)
#endif
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

QString OrcaFlash::statusString(const OrcaFlash::ORCA_STATUS status)
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
    logger->info(QString("Camera status: %1").arg(getStatusString()));
}
