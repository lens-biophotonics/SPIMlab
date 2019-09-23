#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QSerialPort>
#include <QState>

class SerialPort : public QSerialPort
{
    Q_OBJECT
public:
    SerialPort(QObject *parent = nullptr);
    bool open(OpenMode mode = QIODevice::ReadWrite);
    void close();

    void sendMsg(QString msg);
    QString receive();
    QString transceive(QString command);
    double getDouble(const QString &cmd);
    int getInt(const QString &cmd);
    QString getSerialNumber();

    static QSerialPortInfo findPortFromSerialNumber(const QString &sn);

    void setSerialNumber(const QString &serialNumber);
    void setTimeout(int ms);
    QString getLineEndTermination();
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
    int _serialTimeout;

    QState *connectedState;
    QState *disconnectedState;

    void setupStateMachine();
};

#endif // SERIALPORT_H
