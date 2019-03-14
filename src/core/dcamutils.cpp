#include "dcamutils.h"
#include "logger.h"
#include <QMap>

using namespace DCAM;

static Logger *logger = getLogger("DCAM");
static QMap<int, ModelInfo *> map;
static QMap<QString, int> mapByIDStr;


namespace DCAM {
QString getModelInfo(const int index, const int32 dwStringID)
{
    char buf[128];
#ifdef WITH_HARDWARE
    dcam_getmodelinfo(index, dwStringID, buf, 128);
#else
    Q_UNUSED(index)
    Q_UNUSED(dwStringID)
#endif
    return QString(buf);
}

ModelInfo *getModelInfo(const int index)
{
    if (!map.contains(index)) {
        return nullptr;
    }
    return map[index];
}

int getCameraIndex(const QString idStr)
{
    if (!mapByIDStr.contains(idStr)) {
        return -1;
    }
    return mapByIDStr[idStr];
}

int init_dcam()
{
    int nCamera;
#ifdef WITH_HARDWARE
    bool ok = false;
#if DCAM_VERSION == 400
    DCAMAPI_INIT param;
    memset (&param, 0, sizeof(param));

    param.size = sizeof(param);
    int32 ret = dcamapi_init (&param);
    ok = ret == DCAMERR_SUCCESS;
    nCamera = param.iDeviceCount;
#else
    ok = dcam_init(nullptr, &nCamera);
#endif
    if (!ok) {
        QString errMsg = "Cannot initialize dcam";
        logger->critical(errMsg);
        throw std::runtime_error(errMsg.toStdString());
    }

    if (nCamera == 0) {
        QString errMsg = "No cameras found";
        logger->critical(errMsg);
        throw std::runtime_error(errMsg.toStdString());
    }
    logger->info(QString("Found %1 cameras").arg(nCamera));
    for (int i = 0; i < nCamera; ++i) {
        ModelInfo *mi = new ModelInfo();
        mi->vendor = getModelInfo(i, DCAM_IDSTR_VENDOR);
        mi->model = getModelInfo(i, DCAM_IDSTR_MODEL);
        mi->bus = getModelInfo(i, DCAM_IDSTR_BUS);
        mi->cameraID = getModelInfo(i, DCAM_IDSTR_CAMERAID);
        mi->cameraVersion = getModelInfo(i, DCAM_IDSTR_CAMERAVERSION);
        mi->driverVersion = getModelInfo(i, DCAM_IDSTR_DRIVERVERSION);
        logger->info(QString("Camera #%1 ").arg(i)
                     + mi->vendor + " "
                     + mi->model + " "
                     + mi->cameraID + " "
                     + mi->cameraVersion);
        map.insert(i, mi);
        mapByIDStr.insert(mi->cameraID, i);
    }
#else
    nCamera = 8;
#endif
    return nCamera;
}

void uninit_dcam()
{
#ifdef WITH_HARDWARE
#if DCAM_VERSION == 400
    if (dcamapi_uninit() != DCAMERR_SUCCESS)
#else
    if (!dcam_uninit())
#endif
    {
        QString errMsg = "Cannot uninitialize dcam";
        logger->critical(errMsg);
        throw std::runtime_error(errMsg.toStdString());
    }
#endif
    qDeleteAll(map);
    map.clear();
    mapByIDStr.clear();
}
}
