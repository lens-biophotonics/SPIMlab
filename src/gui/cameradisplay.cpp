#include <QTimer>
#include <QHBoxLayout>
#include <QPushButton>

#include <qwt/qwt_slider.h>

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

    QVBoxLayout *layout = new QVBoxLayout();
    QwtSlider *minSlider = new QwtSlider(Qt::Horizontal);
    QwtSlider *maxSlider = new QwtSlider(Qt::Horizontal);

    QGridLayout *grid = new QGridLayout();

    minSlider->setScalePosition(QwtSlider::LeadingScale);
    minSlider->setLowerBound(0);
    minSlider->setUpperBound(65535);
    minSlider->setValue(0);
    minSlider->setEnabled(false);

    maxSlider->setScalePosition(QwtSlider::LeadingScale);
    maxSlider->setLowerBound(0);
    maxSlider->setUpperBound(65535);
    maxSlider->setValue(65535);
    maxSlider->setEnabled(false);

    QPushButton *autoScalePushButton = new QPushButton("Autoscale");
    autoScalePushButton->setSizePolicy(QSizePolicy::Preferred,
                                       QSizePolicy::Expanding);
    autoScalePushButton->setCheckable(true);
    autoScalePushButton->setChecked(true);

    grid->addWidget(minSlider, 0, 0);
    grid->addWidget(maxSlider, 1, 0);
    grid->addWidget(autoScalePushButton, 0, 1, 2, 1);

    layout->addWidget(plot);
    layout->addLayout(grid);

    setLayout(layout);

    connect(minSlider, &QwtSlider::valueChanged, this, [ = ](){
        plot->setInterval(Qt::ZAxis, minSlider->value(), maxSlider->value());
    });

    connect(maxSlider, &QwtSlider::valueChanged, this, [ = ](){
        plot->setInterval(Qt::ZAxis, minSlider->value(), maxSlider->value());
    });

    connect(autoScalePushButton, &QPushButton::clicked, this,
            [ = ](bool checked) {
        minSlider->setEnabled(!checked);
        maxSlider->setEnabled(!checked);
        plot->setZAutoscaleEnabled(checked);
        if (!checked) {
            plot->setInterval(Qt::ZAxis,
                              minSlider->value(), maxSlider->value());
        }
    });
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
    uint16_t *mybuf = new uint16_t[2048 * 2048];
    orca->copyLastFrame(mybuf, 2048 * 2048 * sizeof(uint16_t));
    for (int i = 0; i < 2048 * 2048; ++i) {
        buf[i] = mybuf[i];
    }
    delete[] mybuf;
    emit newImage();
}
