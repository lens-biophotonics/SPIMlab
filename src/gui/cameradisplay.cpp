#include <QHBoxLayout>
#include <QtDebug>
#include <sys/types.h>

#include "core/spimhub.h"
#include "cameradisplay.h"

CameraDisplay::CameraDisplay(QWidget *parent) : QWidget(parent)
{
    timer = new QTimer(this);
    timer->setObjectName("timer");
    timer->setInterval(500);

    setupUi();

    connect(&spimHub(), SIGNAL(captureStarted()), timer, SLOT(start()));
    connect(&spimHub(), SIGNAL(stopped()), timer, SLOT(stop()));

    QMetaObject::connectSlotsByName(this);
}

CameraDisplay::~CameraDisplay()
{
}

void CameraDisplay::setupUi()
{
    plot = new CameraPlot();

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(plot);
    setLayout(layout);
}

void CameraDisplay::on_timer_timeout()
{
    QVector<double> vec(2048 * 2048);
    OrcaFlash *camera = spimHub().camera();
#ifdef WITH_HARDWARE
    void *top;
    int32_t rowBytes;
    camera->lockData(&top, &rowBytes, -1);
    const uint16_t *p = static_cast<const uint16_t *>(top);
    for (int i = 0; i < 2048 * 2048; ++i) {
        vec[i] = p[i];
    }
    camera->unlockData();
#else
    camera->copyLastFrame(vec.data(), 2048 * 2048 * sizeof(double));
#endif

    plot->setData(vec);
}
