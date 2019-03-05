#include <QTimer>
#include <QHBoxLayout>

#include <sys/types.h>

#include "core/spim.h"
#include "cameradisplay.h"


CameraDisplay::CameraDisplay(OrcaFlash *camera, QWidget *parent) :
    QWidget(parent), orca(camera)
{
    setupUi();

    vec.resize(2048 * 2048);

    thread = new QThread(this);
    DisplayWorker *worker = new DisplayWorker(orca, vec.data());
    connect(worker, &DisplayWorker::newImage, this, &CameraDisplay::replot);
    worker->moveToThread(thread);
    thread->start();
}

CameraDisplay::~CameraDisplay()
{
    thread->quit();
}

void CameraDisplay::replot()
{
    plot->setData(vec);
}

void CameraDisplay::setupUi()
{
    plot = new CameraPlot();

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(plot);
    setLayout(layout);
}

DisplayWorker::DisplayWorker(OrcaFlash *camera, double *data, QObject *parent) : QThread(parent)
{
    orca = camera;
    buf = data;
    timer = new QTimer(this);
    timer->setInterval(500);

    void (QTimer::* mySlot)() = &QTimer::start;
    connect(&spim(), &SPIM::captureStarted, timer, mySlot);
    connect(&spim(), &SPIM::stopped, timer, &QTimer::stop);
    connect(timer, &QTimer::timeout, this, &DisplayWorker::updateImage);
}

DisplayWorker::~DisplayWorker() {}

void DisplayWorker::updateImage()
{
#ifdef WITH_HARDWARE
    void *top;
    int32_t rowBytes;
    orca->lockData(&top, &rowBytes, -1);
    const uint16_t *p = static_cast<const uint16_t *>(top);
    for (int i = 0; i < 2048 * 2048; ++i) {
        buf[i] = p[i];
    }
    orca->unlockData();
#else
    orca->copyLastFrame(buf, 2048 * 2048 * sizeof(double));
#endif

    emit newImage();
}
