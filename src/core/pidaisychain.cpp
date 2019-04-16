#include <stdexcept>
#include <memory>

#include <QMap>

#include <PI/PI_GCS2_DLL.h>

#include "pidaisychain.h"


static QMap<QString, PIDaisyChain *> dcMap;


PIDaisyChain::PIDaisyChain(QObject *parent) : QObject(parent), id(-1)
{
}

PIDaisyChain::~PIDaisyChain()
{
    close();
}

void PIDaisyChain::open(const QString &serialPortName, const int baud)
{
    std::unique_ptr<char[]> buf(new char[1024]);
    id = PI_OpenRS232DaisyChainByDevName(serialPortName.toStdString().c_str(),
                                         baud, &_nOfDevices, buf.get(), 1024);

    if (id == -1) {
        throw std::runtime_error("PI_OpenRS232DaisyChainByDevName failed");
    }
}

int PIDaisyChain::connectDevice(const int deviceNumber)
{
    if (deviceNumber < 1 || deviceNumber > _nOfDevices) {
        throw std::runtime_error("Invalid device number");
    }
    int devid = PI_ConnectDaisyChainDevice(id, deviceNumber);
    if (devid == -1) {
        throw std::runtime_error("PI_ConnectDaisyChainDevice failed");
    }
    return devid;
}

void PIDaisyChain::close()
{
    PI_CloseDaisyChain(id);
    id = -1;
}

bool PIDaisyChain::isOpen()
{
    return id != -1;
}

int PIDaisyChain::nOfDevices() const
{
    return _nOfDevices;
}

PIDaisyChain *openDaisyChain(const QString &portName, const int baud)
{
    PIDaisyChain *dc;
    auto it = dcMap.find(portName);
    if (it == dcMap.end()) {
        dc = new PIDaisyChain();
        dc->open(portName, baud);
        dcMap[portName] = dc;
    }
    else {
        dc = it.value();
    }
    return dc;
}

void closeDaisyChain(const QString &portName)
{
    auto it = dcMap.find(portName);
    if (it != dcMap.end()) {
        it.value()->close();
    }
}

void closeAllDaisyChains()
{
    qDeleteAll(dcMap);
    dcMap.clear();
}
