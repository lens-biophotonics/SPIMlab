#include "cobolt.h"
#include "serialport.h"

Cobolt::Cobolt(QObject *parent) : QObject(parent)
{
    serial = new SerialPort(this);

    connect(serial, &SerialPort::opened, this, &Cobolt::connected);
    connect(serial, &SerialPort::closed, this, &Cobolt::disconnected);
}

Cobolt::~Cobolt()
{
}

void Cobolt::open()
{
    serial->open();
    serial->readAll();  // empty input buffer
}

void Cobolt::close()
{
    serial->close();
}

SerialPort *Cobolt::serialPort() const
{
    return serial;
}

QString Cobolt::getSerialNumber()
{
    return serial->transceive("gsn?");
}

/**
 * @brief Get output power set point.
 * @return Output power in W.
 */

float Cobolt::getOutputPower()
{
    return serial->getFloat("pa?");
}

/**
 * @brief Get drive current.
 * @return Drive current in mA.
 */

float Cobolt::getDriveCurrent()
{
    return serial->getFloat("i?");
}

Cobolt::ON_OFF_STATE Cobolt::getOnOffState()
{
    int state = serial->getInt("l?");
    return static_cast<ON_OFF_STATE>(state);
}

Cobolt::OPERATING_MODE Cobolt::getOperatingMode()
{
    int mode = serial->getInt("gom?");
    return static_cast<OPERATING_MODE>(mode);
}
