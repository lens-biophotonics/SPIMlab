#include <QHBoxLayout>
#include <QtDebug>

#include "cameradisplay.h"

CameraDisplay::CameraDisplay(QWidget *parent) : QWidget(parent)
{
    timer = new QTimer(this);
    timer->setObjectName("timer");

    buf = new uint16_t[2048 * 2048];
    vec = QVector<double>(2048 * 2048);

    hub = SPIMHub::getInstance();

    setupUi();

    QMetaObject::connectSlotsByName(this);
}

CameraDisplay::~CameraDisplay()
{
    delete[] buf;
}

void CameraDisplay::startRefreshTimer()
{
    timer->start(100);
}

void CameraDisplay::stopRefreshTimer()
{
    timer->stop();
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
    hub->camera()->copyLastFrame(buf, 2048 * 2048 * 2);

    for (size_t i = 0; i < 2048 * 2048; ++i) {
        vec[i] = buf[i];
    }

    plot->setData(vec);
}
