#include "orcaflash.h"

#include "logmanager.h"

static Logger *dcamLogger = LogManager::getInstance().getLogger("DCAM");
static Logger *orcaLogger = LogManager::getInstance().getLogger("OrcaFlash");

using namespace HAMAMATSU;

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


OrcaFlash::OrcaFlash(QObject *parent) : QObject(parent)
{
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

void OrcaFlash::logLastError(QString label)
{
    if (!label.isEmpty())
        label.append(" ");
    orcaLogger->error(getLastError().prepend(label));
}

bool OrcaFlash::startFreeRun()
{
#ifdef WITH_HARDWARE
    dcam_freeframe(h);
    if (!dcam_precapture(h, DCAM_CAPTUREMODE_SEQUENCE)) {
        logLastError("precapture");
        return false;
    }

    if (!dcam_allocframe(h, 10)) {
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

bool OrcaFlash::copyLastFrame(void *buf, size_t n)
{
#ifdef WITH_HARDWARE
    void *top;
    int32 rowbytes;

    _DWORD dwEvent = DCAMCAP_EVENT_FRAMEREADY;
    if (!dcam_wait(h, &dwEvent, 1000, nullptr)) {
        logLastError("wait");
        return false;
    }

    if (!dcam_lockdata(h, &top, &rowbytes, -1))
    {
        logLastError("lockdata");
        return false;
    }

    memcpy(buf, top, n);

    if (!dcam_unlockdata(h))
    {
        logLastError("unlockdata");
        return false;
    }
#else
    Q_UNUSED(buf)
    Q_UNUSED(n)
#endif
    return true;
}
