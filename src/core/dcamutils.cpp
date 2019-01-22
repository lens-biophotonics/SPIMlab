#include <QString>

#include "dcamutils.h"
#include "logger.h"

static Logger *logger = getLogger("DCAM");


using namespace DCAM;

namespace DCAM {
int init_dcam()
{
    int nCamera;
#ifdef WITH_HARDWARE
    if (!dcam_init(nullptr, &nCamera)) {
        QString errMsg = "Cannot initialize dcam";
        logger->critical(errMsg);
        throw std::runtime_error(errMsg.toStdString());
    }

    if (nCamera == 0) {
        QString errMsg = "No cameras found";
        logger->critical(errMsg);
        throw std::runtime_error(errMsg.toStdString());
    }
#else
    nCamera = 8;
#endif
    logger->info(QString("Found %1 cameras").arg(nCamera));
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
}
