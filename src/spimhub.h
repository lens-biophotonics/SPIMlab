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
    static SPIMHub* getInstance();

// C++ 11
#if __cplusplus >= 201103L
    SPIMHub(SPIMHub const &) = delete;
    void operator=(SPIMHub const &) = delete;
#else
    // C++ 03
    // Dont implement copy constructor and assignment operator
    SPIMHub(SPIMHub const &);
    void operator=(SPIMHub const &);
#endif

    OrcaFlash *camera();
    void setCamera(OrcaFlash *camera);

signals:
    void captureStarted();
    void stopped();

public slots:
    void startFreeRun();
    void startAcquisition();
    void stop();

private:
    SPIMHub();
    static SPIMHub* inst;

    OrcaFlash *orca;
    QThread *thread;
    SaveStackWorker *worker;
};

#endif // SPIMHUB_H
