#include <QSerialPortInfo>
#include <QTimer>
#include <QStateMachine>

#include "logger.h"

#include "serialport.h"

#define RUNTIME_ERROR(what) { \
        logger->error(what); \
        throw std::runtime_error(what); \
} \

static Logger *logger = getLogger("SerialPort");

SerialPort::SerialPort(QObject *parent)
    : QSerialPort(parent)
{
    setupStateMachine();

    setBaudRate(Baud115200);
    setFlowControl(NoFlowControl);
    setParity(NoParity);
    setDataBits(Data8);
    setStopBits(OneStop);
    setTimeout(500);
}

bool SerialPort::open(OpenMode mode)
{
    if (!_serialNumber.isEmpty())
    {
        QSerialPortInfo info = findPortFromSerialNumber(_serialNumber);
        if (!info.portName().isEmpty()) {
            logger->info(QString("Found serial number %1 on %2")
                         .arg(info.serialNumber()).arg(info.portName()));
            setPort(info);
        }
    }
    bool ret = QSerialPort::open(mode);
    if (!ret) {
        return false;
    }

    logger->info(QString("Connected on ").append(portName()));
    ret = open_impl();
    if (ret)
        emit opened();
    return ret;
}

bool SerialPort::open_impl()
{
    return true;
}

QState *SerialPort::getDisconnectedState() const
{
    return disconnectedState;
}

QState *SerialPort::getConnectedState() const
{
    return connectedState;
}

void SerialPort::setupStateMachine()
{
    connectedState = new QState();
    disconnectedState = new QState();

    connectedState->addTransition(this, &SerialPort::closed, disconnectedState);

    disconnectedState->addTransition(this, &SerialPort::opened, connectedState);

    QStateMachine *sm = new QStateMachine();

    sm->addState(connectedState);
    sm->addState(disconnectedState);

    sm->setInitialState(disconnectedState);
    sm->start();
}

QString SerialPort::receiveMsg()
{
    qint64 numBytes;

    numBytes = bytesAvailable();
    if (numBytes > 1024)
        numBytes = 1024;

    QString msg = readAll();

    if (loggingEnabled) {
        logger->info(">>> " + msg);
    }
    return msg.replace(lineEndTermination, "");
}

void SerialPort::sendMsg(QString msg)
{
    msg.append(lineEndTermination);
    if (!isOpen()) {
        RUNTIME_ERROR("Device not open")
    }
    qint64 ret = write(msg.toLatin1());
    if (ret < 0) {
        RUNTIME_ERROR("QSerialPort::write");
    }
    if (loggingEnabled) {
        logger->info("<<< " + msg);
    }
}

QString SerialPort::transceive(QString command)
{
    QString receivedLines;
    sendMsg(command);
    do {
        if (bytesAvailable() <= 0) {
            if (!waitForReadyRead(_transceiveTimeout)) {
                break;
            }
        }
        QString msg = receiveMsg();
        receivedLines.append(msg);
    }
    while (bytesAvailable());

    return receivedLines;
}

void SerialPort::close()
{
    QSerialPort::close();
    if (!isOpen()) {
        emit closed();
        logger->info("Disconnected");
    }
}

QSerialPortInfo SerialPort::findPortFromSerialNumber(const QString &sn)
{
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        if (info.serialNumber() == sn) {
            return info;
        }
    }
    return QSerialPortInfo();
}

/**
 * @brief Convenience function to retrieve a float.
 * @param cmd The command to be sent.
 * @return The retrieved float.
 */

double SerialPort::getDouble(const QString &cmd)
{
    QString str = transceive(cmd);
    bool ok;
    double f = str.toDouble(&ok);
    if (!ok) {
        throw std::runtime_error("Cannot convert string to float");
    }
    return f;
}

/**
 * @brief Convenience function to retrieve an int.
 * @param cmd The command to be sent.
 * @return The retrieved int.
 */

int SerialPort::getInt(const QString &cmd)
{
    QString str = transceive(cmd);
    bool ok;
    int integer = str.toInt(&ok);
    if (!ok) {
        throw std::runtime_error("Cannot convert string to int");
    }
    return integer;
}

void SerialPort::setTimeout(int ms)
{
    _transceiveTimeout = ms;
}

void SerialPort::setSerialNumber(const QString &serialNumber)
{
    _serialNumber = serialNumber;
}

void SerialPort::setLineEndTermination(const QString &termination)
{
    lineEndTermination = termination;
}

void SerialPort::setLoggingEnabled(bool enable)
{
    loggingEnabled = enable;
}
