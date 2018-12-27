#include <QHBoxLayout>
#include <QApplication>

#include "cameradisplay.h"
#include "spimevent.h"

CameraDisplay::CameraDisplay(QWidget *parent) : QWidget(parent)
{
    timer = new QTimer(this);
    timer->setObjectName("timer");

    buf = new uint16_t[2048 * 2048];
    vec = QVector<double>(2048 * 2048);

    setupUi();

    QMetaObject::connectSlotsByName(this);
    qApp->installEventFilter(this);
}

CameraDisplay::~CameraDisplay()
{
    delete[] buf;
}

void CameraDisplay::startRefreshTimer(int msec)
{
    timer->start(msec);
}

void CameraDisplay::stopRefreshTimer()
{
    timer->stop();
}

void CameraDisplay::setCamera(OrcaFlash *camera)
{
    orca = camera;
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
    orca->copyLastFrame(buf, 2048 * 2048);

    for (size_t i = 0; i < 2048 * 2048; ++i) {
        vec[i] = buf[i];
    }

    plot->setData(vec);
}

bool CameraDisplay::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    if (event->type() != SPIMEvent::TYPE) {
        return false;
    }
    SPIMEvent *e = static_cast<SPIMEvent*>(event);
    switch (e->getType()) {
    case SPIMEvent::START_FREE_RUN_REQUESTED:
        startRefreshTimer();
        break;

    case SPIMEvent::STOP_FREE_RUN_REQUESTED:
        stopRefreshTimer();
        break;
    }
    return true;
}
