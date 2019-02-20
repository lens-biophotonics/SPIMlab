#include <stdexcept>
#include <memory>

#include <QString>

#include <PI/PI_GCS2_DLL.h>

#include "pidevice.h"
#include "pidaisychain.h"

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

PIDevice::~PIDevice()
{
    close();
}

void PIDevice::connect(const QString &portName, const int baud)
{
    id = PI_ConnectRS232ByDevName(portName.toStdString().c_str(), baud);

    if (id == -1) {
        throw std::runtime_error("PI_ConnectRS232ByDevName failed");
    }
}

void PIDevice::connectDaisyChain(
    const QString &portName, const int deviceNumber)
{
    id = openDaisyChain(portName, 38400)->connectDevice(deviceNumber);
}

void PIDevice::close()
{
    PI_CloseConnection(id);
    id = -1;
}

bool PIDevice::isConnected()
{
#ifdef WITH_HARDWARE
    return PI_IsConnected(id);
#else
    return id == -1;
#endif
}


void PIDevice::move(const QString &axesStr, const double pos[])
{
#ifndef WITH_HARDWARE
    Q_UNUSED(axesStr)
    Q_UNUSED(pos)
#endif
    CALL_THROW(PI_MOV(id, axesStr.toLatin1(), pos));
}

QStringList PIDevice::getAvailableStageTypes()
{
    std::unique_ptr<char[]> buf(new char[4096]);
    CALL_THROW(PI_qVST(id, buf.get(), 4096));
    QStringList sl = QString(buf.get()).replace(" ", "").split("\n");
    sl.removeAll(QString(""));
    return sl;
}

QString PIDevice::getErrorString()
{
    std::unique_ptr<char[]> buf(new char[1024]);
    PI_TranslateError(PI_GetError(id), buf.get(), 1024);
    return QString(buf.get());
}
