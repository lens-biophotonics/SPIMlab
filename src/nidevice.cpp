#include "nidevice.h"


NIDevice::NIDevice() :
    QObject()
{
    errBuff = (char*)calloc(sizeof(char), 2048);
}

NIDevice::~NIDevice() {
    free(errBuff);
}

/**
 * @brief Recovers the string description of the last occurred error
 */

void NIDevice::onError() {
#ifdef WITH_HARDWARE
    DAQmxGetExtendedErrorInfo(errBuff, 2048);
#endif
    emit errorOccurred(QString("DAQmx Error: %1").arg(errBuff));
}
