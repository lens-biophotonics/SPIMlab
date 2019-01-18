#include <QTimer>
#include <QHBoxLayout>

#include <sys/types.h>

#include "core/spim.h"
#include "cameradisplay.h"


CameraDisplay::CameraDisplay(QWidget *parent) : QWidget(parent)
{
    setupUi();

    vec.resize(2048 * 2048);

    thread = new QThread(this);
    DisplayWorker *worker = new DisplayWorker(vec.data());
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

DisplayWorker::DisplayWorker(double *data, QObject *parent) : QThread(parent)
{
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
    OrcaFlash *camera = spim().camera();
#ifdef WITH_HARDWARE
    void *top;
    int32_t rowBytes;
    camera->lockData(&top, &rowBytes, -1);
    const uint16_t *p = static_cast<const uint16_t *>(top);
    for (int i = 0; i < 2048 * 2048; ++i) {
        buf[i] = p[i];
    }
    camera->unlockData();
#else
    camera->copyLastFrame(buf, 2048 * 2048 * sizeof(double));
#endif

    emit newImage();
}
