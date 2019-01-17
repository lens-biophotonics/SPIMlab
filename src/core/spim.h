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
    SPIM(QObject *parent=nullptr);
    virtual ~SPIM();

    void initialize();
    void uninitialize();

    OrcaFlash *camera();
    void setCamera(OrcaFlash *camera);

    void setupCameraTrigger(QString COPhysicalChan, QString terminal);

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
    QThread *thread = nullptr;
    SaveStackWorker *worker = nullptr;
    CameraTrigger *cameraTrigger = nullptr;
    GalvoRamp *galvoRamp = nullptr;

    void setExposureTime(double expTime);
};

SPIM& spim();

#endif // SPIMHUB_H
