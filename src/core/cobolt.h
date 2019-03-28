#ifndef COBOLT_H
#define COBOLT_H

#include <QObject>

class SerialPort;

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

    enum OPERATING_FAULT {
        FAULT_NOERRORS = 0,
        FAULT_TEMPERATUR_EERROR = 1,
        FAULT_INTERLOCK_ERROR = 3,
        FAULT_CONSTANT_POWER_TIMEOUT= 4,
    };

public:
    Cobolt(QObject *parent = nullptr);
    virtual ~Cobolt();

    SerialPort *getSerialPort() const;

    QString getSerialNumber();
    QString getLaserModel();
    QString getFullName();
    int getWavelength();
    double getOutputPower();
    double getOutputPowerSetPoint();
    double getDriveCurrent();
    double getHours();
    double getModulationHighCurrent();
    double getModulationLowCurrent();
    double getTECLDSetTemperature();
    double readTECLDTemperature();
    OPERATING_FAULT getOperatingFault();
    bool isInterlockOpen();
    bool isAnalogModulationEnabled();
    bool isDigitalModulationEnabled();
    bool isAnalogLowImpedanceEnabled();
    ON_OFF_STATE getOnOffState();
    OPERATING_MODE getOperatingMode();

    QString getVerboseName() const;
    void setVerboseName(const QString &value);

public slots:
    void open();
    void close();

    void setOutputPower(double W);
    void setDriveCurrent(double mA);
    void setModulationHighCurrent(double mA);
    void setModulationLowCurrent(double mA);
    void setTECLDTemperature(double celsius);
    void setDigitalModulationEnabled(bool enable);
    void setAnalogModulationEnabled(bool enable);
    void setAnalogLowImpedanceEnabled(bool enable);

    void restart();
    void setLaserOn();
    void setLaserOff();
    void enterConstantPowerMode();
    void enterConstantCurrentMode();
    void enterModulationMode();
    void clearFault();
    void ping();

signals:
    void connected();
    void disconnected();

private:
    SerialPort *serial = nullptr;
    QString verboseName;

    QString transceiveChkOK(QString cmd);
    QString transceiveChkSyntaxError(QString cmd);
};

#endif // COBOLT_H
