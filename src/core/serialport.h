#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QSerialPort>
#include <QState>

class SerialPort : public QSerialPort
{
    Q_OBJECT
public:
    SerialPort(QObject *parent = nullptr);
    void sendMsg(QString msg);
    void close();

    static QSerialPortInfo findPortFromSerialNumber(const QString &sn);

    QString transceive(QString command);
    bool open(OpenMode mode = QIODevice::ReadWrite);

    float getFloat(const QString &cmd);
    int getInt(const QString &cmd);

    void setSerialNumber(const QString &serialNumber);
    void setTimeout(int ms);
    void setLineEndTermination(const QString &termination);
    void setLoggingEnabled(bool enable);

    QState *getConnectedState() const;
    QState *getDisconnectedState() const;

private slots:
    QString receiveMsg();

signals:
    void opened();
    void closed();

protected:
    virtual bool open_impl();

private:
    QString _serialNumber;
    QString lineEndTermination;
    bool loggingEnabled = false;
    int _transceiveTimeout;

    QState *connectedState;
    QState *disconnectedState;

    void setupStateMachine();
};

#endif // SERIALPORT_H
