#ifndef DCAMUTILS_H
#define DCAMUTILS_H

#include <QString>

#ifndef DCAM_VERSION
#define DCAM_VERSION 400
#endif

namespace DCAM {
#ifdef DCAMAPI_HEADERS
#include <dcamapi.h>
#include <dcamprop.h>
#endif

struct ModelInfo {
    QString vendor;
    QString model;
    QString bus;
    QString cameraID;
    QString cameraVersion;
    QString driverVersion;
};

int init_dcam();
void uninit_dcam();

ModelInfo *getModelInfo(const int index);
int getCameraIndex(const QString idStr);
}

#endif // DCAMUTILS_H
