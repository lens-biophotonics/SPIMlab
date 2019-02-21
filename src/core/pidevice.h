#ifndef PIDEVICE_H
#define PIDEVICE_H

#include <PI/PI_GCS2_DLL.h>


#include <QObject>
#include <QState>


class PIDevice : public QObject
{
    Q_OBJECT

public:
    explicit PIDevice(QObject *parent = nullptr);
    virtual ~PIDevice();
    void connectSerial(const QString &portName, const int baud);
    void connectDaisyChainSerial(const QString &portName,
                                 const int deviceNumber, const int baud = 38400);
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

    int deviceNumber() const;
    QString portName() const;
    int baudRate() const;

    QState *connectedState() const;
    QState *disconnectedState() const;

signals:
    void connected();
    void disconnected();

public slots:
    void close();

private:
    void setupStateMachine();

    int id;
    int _deviceNumber = 1;
    QString _portName;
    QState *_connectedState = nullptr;
    QState *_disconnectedState = nullptr;
    int _baud = -1;
};

#endif // PIDEVICE_H
