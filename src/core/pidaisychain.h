#ifndef PIDAISYCHAIN_H
#define PIDAISYCHAIN_H

#include <QObject>
#include <QList>

#include "pidevice.h"

class PIDaisyChain : public QObject
{
    Q_OBJECT
public:
    explicit PIDaisyChain(QObject *parent = nullptr);
    virtual ~PIDaisyChain();

    void open(const QString &serialPortName, const int baud);
    int connectDevice(const int deviceNumber);
    void close();

    bool isOpen();
    int nOfDevices() const;

signals:

public slots:

private:
    int id;
    int _nOfDevices;
};

PIDaisyChain *openDaisyChain(const QString &portName, const int baud = 115200);
void closeDaisyChain(const QString &portName);
void closeAllDaisyChains();

#endif // PIDAISYCHAIN_H
