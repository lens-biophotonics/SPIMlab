#ifndef SPIMHUB_H
#define SPIMHUB_H

#include <QObject>
#include <QThread>

#include "savestackworker.h"
#include "orcaflash.h"

class SPIMHub : public QObject
{
    Q_OBJECT
public:
    SPIMHub();
    virtual ~SPIMHub();

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
    OrcaFlash *orca;
    QThread *thread;
    SaveStackWorker *worker;
};

SPIMHub& spimHub();

#endif // SPIMHUB_H
