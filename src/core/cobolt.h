#ifndef COBOLT_H
#define COBOLT_H

#include <QObject>

#include "serialport.h"

class Cobolt : public QObject
{
    Q_OBJECT

    enum ON_OFF_STATE {
        STATE_OFF = 0,
        STATE_ON = 1,
    };

    enum OPERATING_MODE {
        MODE_OFF = 0,
        MODE_WAITING_KEY = 1,
        MODE_CONTINUOUS = 2,
        MODE_ON_OFF_MODULATION = 3,
        MODE_MODULATION = 4,
        MODE_FAULT = 5,
        MODE_ABORTED = 6,
    };

public:
    Cobolt(QObject *parent = nullptr);
    virtual ~Cobolt();

    SerialPort *serialPort() const;

    QString getSerialNumber();
    float getOutputPower();
    float getDriveCurrent();
    ON_OFF_STATE getOnOffState();
    OPERATING_MODE getOperatingMode();

public slots:
    void open();
    void close();

signals:
    void connected();
    void disconnected();

private:
    SerialPort *serial = nullptr;

    float getFloat(const QString &cmd);
    int getInt(const QString &cmd);
};

#endif // COBOLT_H
