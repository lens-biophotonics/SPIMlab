#include <QRegularExpression>
#include <QStringList>
#include <QtDebug>

#include "filterwheel.h"
#include "serialport.h"


FilterWheel::FilterWheel(QObject *parent) : QObject(parent)
{
    serial = new SerialPort(this);
    serial->setLineEndTermination("\r");
    positionCount = -1;
    setMotionTime(FILTERWHEEL_MOTION_TIME); // wait in ms between reaching any position
}

FilterWheel::~FilterWheel()
{
    close();
}

void FilterWheel::open()
{
    bool ret = serial->open();
    if (!ret)
        throw std::runtime_error(QString("Cannot connect to Filter Wheel with serial number " + serial->getSerialNumber()).toLatin1());
    else {
        serial->readAll();  // empty input buffer
        getPositionCount();
        emit connected();
    }
}

void FilterWheel::close()
{
    serial->close();
    emit disconnected();
}

SerialPort *FilterWheel::getSerialPort() const
{
    return serial;
}

QString FilterWheel::getID()
{
    return transceiveChkSyntaxError("*idn?");
}

FilterWheel::SERIAL_BAUD_RATE FilterWheel::getBaudRate()
{
    QString str = transceiveChkSyntaxError("baud?");
    return static_cast<SERIAL_BAUD_RATE>(castStringToIntChkError(str));
}

int FilterWheel::getPosition()
{
    QString str = transceiveChkSyntaxError("pos?");
    return castStringToIntChkError(str);
}

/**
 * @brief FilterWheel::getPositionCount
 * @return number of filter slots (either 6 or 12)
 */

int FilterWheel::getPositionCount()
{
    if (positionCount < 0) {
        QString str = transceiveChkSyntaxError("pcount?");
        positionCount = castStringToIntChkError(str);
    }
    return positionCount;
}

int FilterWheel::getMotionTime() const
{
    return motionTime; // ms
}

void FilterWheel::setMotionTime(const int value)
{
    motionTime = value; // ms
}

/**
 * @brief Query whether wheel sensor is active or not when idle
 * @return
 */

FilterWheel::IDLE_STATE_SENSOR_STATE FilterWheel::getSensorMode()
{
    QString str = transceiveChkSyntaxError("sensors?");
    return static_cast<IDLE_STATE_SENSOR_STATE>(castStringToIntChkError(str));
}

FilterWheel::SPEED_MODE FilterWheel::getSpeedMode()
{
    QString str = transceiveChkSyntaxError("speed?");
    return static_cast<SPEED_MODE>(castStringToIntChkError(str));
}

FilterWheel::TRIGGER_MODE FilterWheel::getTriggerMode()
{
    QString str = transceiveChkSyntaxError("trig?");
    return static_cast<TRIGGER_MODE>(castStringToIntChkError(str));
}

void FilterWheel::restoreDefaultSettings()
{
    setBaudRate(BAUD_115200);
    setSensorMode(IDLE_STATE_SENSOR_OFF);
    setSpeedMode(SPEED_MODE_HIGH);
    setTriggerMode(TRIGGER_MODE_OUT);
    transceiveChkSyntaxError(QString("save"));
}

/**
 * @brief Save current settings as default on device power-up.
 */

void FilterWheel::saveSettings()
{
    transceiveChkSyntaxError(QString("save"));
}

void FilterWheel::setBaudRate(FilterWheel::SERIAL_BAUD_RATE rate)
{
    transceiveChkSyntaxError(QString("baud=%1").arg(rate));
}

QString FilterWheel::setPosition(int n)
{
    if (n < 1 || n > positionCount)
        return QString("ErrorFilterWheel: requested position %1 out of range [1,%2]").arg(n).arg(positionCount);
    else
        return transceiveChkSyntaxError(QString("pos=%1").arg(n));
}

void FilterWheel::setPositionCount(int n)
{
    transceiveChkSyntaxError(QString("pcount=%1").arg(n));
}

void FilterWheel::setSensorMode(FilterWheel::IDLE_STATE_SENSOR_STATE status)
{
    transceiveChkSyntaxError(QString("sensors=%1").arg(status));
}

void FilterWheel::setSpeedMode(FilterWheel::SPEED_MODE status)
{
    transceiveChkSyntaxError(QString("speed=%1").arg(status));
}

void FilterWheel::setTriggerMode(FilterWheel::TRIGGER_MODE status)
{
    transceiveChkSyntaxError(QString("trig=%1").arg(status));
}

void FilterWheel::ping()
{
    transceiveChkSyntaxError("?");
}

QString FilterWheel::transceiveChkSyntaxError(QString cmd)
{
    serial->sendMsg(cmd);
    removeEcho(cmd);
    QString response = serial->receive();
    response.append(serial->receive()); // sometimes the device does not send the full response during one communication
    response.remove('\r');
    response.remove(QRegExp("[<>]"));
    if (response.startsWith("Command error")) {
        throw std::runtime_error(response.toLatin1());
    }
    return response;
}

void FilterWheel::removeEcho(QString cmd)
{
    QString cmdecho = QString();
    do {
        QString msg = serial->receive();
        cmdecho.append(msg);
    } while (!cmdecho.contains(cmd, Qt::CaseSensitive));
}

int FilterWheel::castStringToIntChkError(QString str)
{
    bool ok;
    int integer = str.toInt(&ok);
    if (!ok) {
        throw std::runtime_error(
                  QString("Cannot convert string to int: " + str).toLatin1());
    }
    return integer;
}
