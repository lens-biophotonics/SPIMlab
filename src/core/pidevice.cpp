#include <stdexcept>
#include <memory>

#include <QString>

#include <PI/PI_GCS2_DLL.h>

#include "pidevice.h"

#define FUNCNAME(x) # x
#ifdef WITH_HARDWARE
#define CALL_THROW(functionCall) \
    if (!functionCall) { \
        throw std::runtime_error(std::string(FUNCNAME(functionCall)) + std::string(" error")); \
    }
#else
#define CALL_THROW(func)
#endif


PIDevice::PIDevice(QObject *parent) : QObject(parent)
{
}

void PIDevice::openDaisyChain(
    const QString &description, const int deviceNumber)
{
    std::unique_ptr<char[]> buf(new char[2048]);
    int nOfConnectedDCDevices;
    int id;

    id = PI_OpenUSBDaisyChain(
        description.toLatin1(), &nOfConnectedDCDevices, buf.get(), 2048);
    if (id == -1) {
        throw std::runtime_error("PI_OpenUSBDaisyChain failed");
    }
    if (deviceNumber < 1 || deviceNumber > nOfConnectedDCDevices)
    {
        throw std::runtime_error("Invalid deviceNumber");
    }
    id = PI_ConnectDaisyChainDevice(id, deviceNumber);
    if (id == -1) {
        throw std::runtime_error("PI_ConnectDaisyChainDevice failed");
    }
    this->id = id;
}

void PIDevice::move(const QString &axesStr, const double pos[])
{
#ifndef WITH_HARDWARE
    Q_UNUSED(axesStr)
    Q_UNUSED(pos)
#endif
    CALL_THROW(PI_MOV(id, axesStr.toLatin1(), pos))
}
