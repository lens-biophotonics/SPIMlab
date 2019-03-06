#ifndef PIDEVICE_H
#define PIDEVICE_H

#include <PI/PI_GCS2_DLL.h>


#include <QObject>
#include <QState>
#include <QMutex>


class PIDevice : public QObject
{
    Q_OBJECT

public:
    explicit PIDevice(QObject *parent = nullptr);
    explicit PIDevice(const QString &verboseName, QObject *parent = nullptr);
    virtual ~PIDevice();
    void connectSerial(const QString &portName, const int baud);
    void connectDaisyChainSerial(const QString &portName,
                                 const int deviceNumber, const int baud = 38400);

    void connectDevice();

    bool isConnected();

    void move(const QString &axes, const double pos[]);
    void setServoEnabled(const QString &axes, const QVector<int> &enable);

    void fastMoveToPositiveLimit(const QString &axes = "");
    void fastMoveToNegativeLimit(const QString &axes = "");
    void fastMoveToReferenceSwitch(const QString &axes = "");
    void loadStages(const QString &axes, const QStringList &stages);
    QStringList getStages(const QString &axes = "", bool stripAxes = false);

    QStringList getAvailableStageTypes();
    QString getAxisIdentifiers();
    QVector<int> getReferencedState(QString axes = "");

    QString getErrorString();

    QState *connectedState() const;
    QState *disconnectedState() const;

    int getDeviceNumber() const;
    void setDeviceNumber(int value);

    int getBaud() const;
    void setBaud(int value);

    QString getPortName() const;
    void setPortName(const QString &value);

    QString getVerboseName() const;
    void setVerboseName(const QString &value);

signals:
    void connected();
    void disconnected();

public slots:
    void close();

private:
    void setupStateMachine();

    int id;

    int deviceNumber = -1;
    int baud = -1;
    QString portName;

    QState *_connectedState = nullptr;
    QState *_disconnectedState = nullptr;

    QString verboseName;
    QMutex mutex;
};

#endif // PIDEVICE_H
