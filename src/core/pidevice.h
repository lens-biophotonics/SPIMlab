#ifndef PIDEVICE_H
#define PIDEVICE_H

#include <QObject>

class PIDevice : public QObject
{
    Q_OBJECT

public:
    explicit PIDevice(QObject *parent = nullptr);
    void openDaisyChain(const QString &description, const int deviceNumber);

    void move(const QString &axesStr, const double pos[]);

signals:

public slots:

private:
    int id;
};

#endif // PIDEVICE_H
