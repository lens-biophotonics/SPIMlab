#ifndef PIDEVICE_H
#define PIDEVICE_H

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
    void move(const QString &axesStr, const double pos[]);

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
