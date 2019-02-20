#ifndef PIDEVICE_H
#define PIDEVICE_H

#include <PI/PI_GCS2_DLL.h>


#include <QObject>


class PIDevice : public QObject
{
    Q_OBJECT

public:
    explicit PIDevice(QObject *parent = nullptr);
    virtual ~PIDevice();
    void connect(const QString &portName, const int baud);
    void connectDaisyChain(const QString &portName,
                           const int deviceNumber);
    void close();

    bool isConnected();

    void move(const QString &axes, const double pos[]);
    void setServoEnabled(const QString &axes, const QVector<int> &enable);

    void fastMoveToPositiveLimit(const QString &axes = "");
    void fastMoveToNegativeLimit(const QString &axes = "");
    void fastMoveToReferenceSwitch(const QString &axes = "");
    void loadStages(const QString &axes, const QStringList &stages);
    QStringList getStages(const QString &axes = "");

    QStringList getAvailableStageTypes();
    QString getAxisIdentifiers();
    QVector<int> getReferencedState(QString axes = "");

    QString getErrorString();

signals:

public slots:

private:
    int id;
};

#endif // PIDEVICE_H
