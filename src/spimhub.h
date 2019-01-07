#ifndef SPIMHUB_H
#define SPIMHUB_H

#include <boost/signals2/signal.hpp>
#include <boost/thread.hpp>

#include "orcaflash.h"

namespace bs2 = boost::signals2;

typedef bs2::signal<void ()> simpleSignal_t;

class SPIMHub
{
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

    void startFreeRun();
    void startAcquisition();
    void stop();
    void requestStop();

    simpleSignal_t captureStarted;
    simpleSignal_t stopped;

private:
    SPIMHub();
    static SPIMHub* inst;

    OrcaFlash *orca;
    boost::thread *thread;
    bool stopRequested;

    void worker(uint framecount);
};

#endif // SPIMHUB_H
