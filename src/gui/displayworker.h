#ifndef DISPLAYWORKER_H
#define DISPLAYWORKER_H

#include <QThread>

class CameraDisplay;

class OrcaFlash;

class DisplayWorker : public QThread
{
    Q_OBJECT
public:
    DisplayWorker(OrcaFlash *orca, CameraDisplay *cd, QObject *parent = nullptr);
    virtual ~DisplayWorker();

signals:
    void newImage(double *data, size_t n);

protected:
    virtual void run();

private:
    OrcaFlash *orca;
    CameraDisplay *cd;
    uint16_t *mybuf;
    bool running;
};

#endif // DISPLAYWORKER_H
