#ifndef PIDEVICE_H
#define PIDEVICE_H

#include <memory>

#include <PI/PI_GCS2_DLL.h>

#include <QObject>
#include <QState>
#include <QMutex>
#include <QVector>

typedef BOOL (*PI_qVectorOfDoubles)(int, const char*, double*);
typedef BOOL (*PI_vectorOfDoubles)(int, const char*, const double*);

class PIDevice : public QObject
{
    Q_OBJECT

public:
    enum TrigOutID {
        OUTPUT_1 = 1,
        OUTPUT_2 = 2,
        OUTPUT_3 = 3,
        OUTPUT_4 = 4,
    };

    enum CTOPam {
        TriggerStep         = 1,
        Axis                = 2,
        TriggerMode         = 3,
        Polarity            = 7,
        StartThreshold      = 8,
        StopThreshold       = 9,
        TriggerPosition     = 10,
    };

    enum TriggerMode {
        PositionDistance    = 0,
        OnTarget            = 2,
        MotionError         = 5,
        InMotion            = 6,
        PositionOffset      = 7,
    };

    enum Polarity {
        ACTIVE_LOW = 0,
        ACTIVE_HIGH = 1,
    };

    explicit PIDevice(QObject *parent = nullptr);
    explicit PIDevice(const QString &verboseName, QObject *parent = nullptr);
    virtual ~PIDevice();
    void connectSerial(const QString &portName, const int baud);
    void connectDaisyChainSerial(const QString &portName,
                                 const int deviceNumber, const int baud = 38400);

    void connectDevice();

    bool isConnected();

    void move(const QString &axes, const double pos[]);
    void move(const double pos[]);
    void move(const double pos);
    void stepUp(const QString &axes);
    void stepDown(const QString &axes);
    void moveRelative(const QString &axes, const double pos[]);
    void setServoEnabled(const QString &axes, const QVector<int> &enable);
    void setServoEnabled(bool enable);
    void halt(const QString &axes = "");
    void setVelocities(const QString &axes, const double vel[]);
    void setVelocities(const double vel[]);
    void setVelocity(const double vel);
    QVector<double> getVelocities(const QString &axes = "");

    QVector<double> getTravelRangeLowEnd(const QString &axes = "");
    QVector<double> getTravelRangeHighEnd(const QString &axes = "");
    QVector<double> getCurrentPosition(const QString &axes = "");
    QVector<double> getCommandedPosition(const QString &axes = "");

    void fastMoveToPositiveLimit(const QString &axes = "");
    void fastMoveToNegativeLimit(const QString &axes = "");
    void fastMoveToReferenceSwitch(const QString &axes = "");
    void loadStages(const QString &axes, const QStringList &stages);
    QStringList getStages(const QString &axes = "", bool stripAxes = false);

    QStringList getAvailableStageTypes();
    QString getAxisIdentifiers();
    QVector<int> getReferencedState(QString axes = "");

    int getError();
    QString getErrorString();

    QState *getConnectedState() const;
    QState *getDisconnectedState() const;

    int getDeviceNumber() const;
    void setDeviceNumber(int value);

    int getBaud() const;
    void setBaud(int value);

    QString getPortName() const;
    void setPortName(const QString &value);

    QString getVerboseName() const;
    void setVerboseName(const QString &value);

    double getStepSize(const QString &axis) const;
    void setStepSize(const QString &axis, const double value);

    void setTriggerOutput(const TrigOutID oid,
                          const CTOPam pam,
                          const double val);
    void setTriggerOutputEnabled(const TrigOutID oid,
                                 const BOOL enable);

    bool isOnTarget(const QString &axis = "");

signals:
    void connected();
    void disconnected();
    void newPositions(const QString &axes, const QVector<double> &pos);
    void onTarget(const QString &axis);

public slots:
    void close();

private:
    void setupStateMachine();
    std::unique_ptr<QVector<double>> getVectorOfDoubles(
        const PI_qVectorOfDoubles fp, const QString &axes);
    void callFunctionWithVectorOfDoubles(
        PI_vectorOfDoubles fp, const QString &axes, const double values[]);

    int id = -1;

    int deviceNumber = -1;
    int baud = -1;
    QString portName;
    QString axisIdentifiers;

    QState *connectedState = nullptr;
    QState *disconnectedState = nullptr;

    QString verboseName;
    QMutex mutex;

    QMap<QString, double> stepSizeMap;
};

#endif // PIDEVICE_H
