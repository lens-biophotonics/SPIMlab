#include "cobolt.h"

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

float Cobolt::getFloat(const QString &cmd)
{
    QString str = serial->transceive(cmd);
    bool ok;
    float f = str.toFloat(&ok);
    if (!ok) {
        throw std::runtime_error("Cannot convert string to float");
    }
    return f;
}

int Cobolt::getInt(const QString &cmd)
{
    QString str = serial->transceive(cmd);
    bool ok;
    int integer = str.toInt(&ok);
    if (!ok) {
        throw std::runtime_error("Cannot convert string to int");
    }
    return integer;
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
    return getFloat("pa?");
}

/**
 * @brief Get drive current.
 * @return Drive current in mA.
 */

float Cobolt::getDriveCurrent()
{
    return getFloat("i?");
}

Cobolt::ON_OFF_STATE Cobolt::getOnOffState()
{
    int state = getInt("l?");
    return static_cast<ON_OFF_STATE>(state);
}

Cobolt::OPERATING_MODE Cobolt::getOperatingMode()
{
    int mode = getInt("gom?");
    return static_cast<OPERATING_MODE>(mode);
}
