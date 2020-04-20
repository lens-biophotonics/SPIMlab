#ifndef DISPLAYWORKER_H
#define DISPLAYWORKER_H

#include <QThread>

class OrcaFlash;

class DisplayWorker : public QThread
{
    Q_OBJECT
public:
    DisplayWorker(OrcaFlash *orca, double *data, QObject *parent = nullptr);
    virtual ~DisplayWorker();

signals:
    void newImage();

protected:
    virtual void run();

private:
    OrcaFlash *orca;
    double *buf;
    uint16_t *mybuf;
    bool running;
};

#endif // DISPLAYWORKER_H
