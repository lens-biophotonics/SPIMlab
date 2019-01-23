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
    SPIM(QObject *parent = nullptr);
    virtual ~SPIM();

    OrcaFlash *camera() const;
    void setCamera(OrcaFlash *camera);

    void setupCameraTrigger(const QString &COPhysicalChan, const QString &terminal);

public slots:
    void startFreeRun();
    void startAcquisition();
    void stop();
    void initialize();
    void uninitialize();

signals:
    void initialized() const;
    void captureStarted() const;
    void stopped() const;
    void error(const QString) const;

private:
    OrcaFlash *orca = nullptr;
    QThread *thread = nullptr;
    SaveStackWorker *worker = nullptr;
    CameraTrigger *cameraTrigger = nullptr;
    GalvoRamp *galvoRamp = nullptr;

    void setExposureTime(double expTime);

private slots:
    void onError(const QString &errMsg) const;
};

SPIM& spim();

#endif // SPIMHUB_H
