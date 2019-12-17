#ifndef FILTERWHEEL_H
#define FILTERWHEEL_H

#define FILTERWHEEL_MOTION_TIME 3000

#include <QObject>

class SerialPort;

class FilterWheel : public QObject
{
    Q_OBJECT

public:
    enum IDLE_STATE_SENSOR_STATE {
        IDLE_STATE_SENSOR_OFF = 0,
        IDLE_STATE_SENSOR_ON = 1,
    };
    Q_ENUM(IDLE_STATE_SENSOR_STATE)

    enum SPEED_MODE {
        SPEED_MODE_SLOW = 0,
        SPEED_MODE_HIGH = 1,
    };
    Q_ENUM(SPEED_MODE)

    enum TRIGGER_MODE {
        TRIGGER_MODE_IN = 0,
        TRIGGER_MODE_OUT = 1,
    };
    Q_ENUM(TRIGGER_MODE)

    enum SERIAL_BAUD_RATE {
        BAUD_9600 = 0,
        BAUD_115200 = 1,
    };
    Q_ENUM(SERIAL_BAUD_RATE)

    FilterWheel(QObject *parent = nullptr);
    virtual ~FilterWheel();

    SerialPort *getSerialPort() const;

    QString getID();
    FilterWheel::SERIAL_BAUD_RATE getBaudRate();
    int getPosition();
    int getPositionCount();
    int getMotionTime() const;
    QStringList getFilterListName() const;
    QString getFilterName(int pos);
    FilterWheel::IDLE_STATE_SENSOR_STATE getSensorMode();
    FilterWheel::SPEED_MODE getSpeedMode();
    FilterWheel::TRIGGER_MODE getTriggerMode();

public slots:
    void open();
    void close();

    void restoreDefaultSettings();
    void saveSettings();
    void setBaudRate(FilterWheel::SERIAL_BAUD_RATE n);
    QString setPosition(int n);
    void setPositionCount(int n);
    void setSensorMode(FilterWheel::IDLE_STATE_SENSOR_STATE status);
    void setSpeedMode(FilterWheel::SPEED_MODE status);
    void setTriggerMode(FilterWheel::TRIGGER_MODE status);

signals:
    void connected();
    void disconnected();

private:
    SerialPort *serial = nullptr;
    QString verboseName;
    int positionCount;
    int motionTime; // ms

    void ping();
    void setMotionTime(const int value);
    QString transceiveChkOK(QString cmd);
    QString transceiveChkSyntaxError(QString cmd);
    void removeEcho(QString cmd);
    int castStringToIntChkError(QString str);
};

#endif // FILTERWHEEL_H
