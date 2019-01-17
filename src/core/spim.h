#ifndef SPIMHUB_H
#define SPIMHUB_H

#include <QObject>
#include <QThread>

#include "savestackworker.h"
#include "orcaflash.h"
#include "cameratrigger.h"
#include "galvoramp.h"

class SPIM : public QObject
{
    Q_OBJECT
public:
    SPIM();
    virtual ~SPIM();

    void initialize();

    OrcaFlash *camera();
    void setCamera(OrcaFlash *camera);

signals:
    void initialized();
    void captureStarted();
    void stopped();

public slots:
    void startFreeRun();
    void startAcquisition();
    void stop();

private:
    OrcaFlash *orca = nullptr;
    QThread *thread;
    SaveStackWorker *worker;
    CameraTrigger *cameraTrigger = nullptr;
    GalvoRamp *galvoRamp = nullptr;
    void setExposureTime(double expTime);
};

SPIM& spim();

#endif // SPIMHUB_H
