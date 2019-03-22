#include <QTimer>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDirIterator>
#include <QMenu>
#include <QStack>
#include <QRegularExpression>

#include <qwt/qwt_slider.h>
#include <qwt/qwt_plot_zoomer.h>

#include "core/spim.h"

#include "aspectratiowidget.h"
#include "cameradisplay.h"
#include "colormaps.h"
#include "settings.h"


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
    QwtPlotZoomer *zoomer = new QwtPlotZoomer(plot->canvas());
    zoomer->setRubberBandPen(QColor(Qt::green));
    zoomer->setTrackerPen(QColor(Qt::green));

    // RightButton: zoom out by 1
    // Ctrl+RightButton: zoom out to full size
    zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
                            Qt::RightButton, Qt::ControlModifier);
    zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
                            Qt::RightButton);


    QVBoxLayout *layout = new QVBoxLayout();
    QwtSlider *minSlider = new QwtSlider(Qt::Horizontal);
    QwtSlider *maxSlider = new QwtSlider(Qt::Horizontal);

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

    QStack<QString> stack;
    QString LUTPath = settings().value(SETTINGSGROUP_OTHERSETTINGS,
                                       SETTING_LUTPATH).toString();
    if (!LUTPath.endsWith(QDir::separator())) {
        LUTPath.append(QDir::separator());
    }
    stack.push(LUTPath);
    QStringList sl;

    QMenu *LUTMenu = new QMenu();
    QAction *action;

    action = new QAction("Grayscale");
    LUTMenu->addAction(action);
    connect(action, &QAction::triggered, this, [ = ](){
        plot->setColorMap(new GrayScaleColorMap());
    });

    action = new QAction("Black Blue White");
    LUTMenu->addAction(action);
    connect(action, &QAction::triggered, this, [ = ](){
        plot->setColorMap(new BlueWhiteColorMap());
    });

    action = new QAction("Hi Low");
    LUTMenu->addAction(action);
    connect(action, &QAction::triggered, this, [ = ](){
        plot->setColorMap(new HiLowColorMap());
    });

    LUTMenu->addSeparator();

    QDirIterator it(LUTPath,
                    QStringList() << "*.lut",
                    QDir::NoFilter, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        sl << it.next();
    }
    sl.sort();

    QStringListIterator sli(sl);
    while (sli.hasNext()) {
        QString path = sli.next();
        QString name = path;
        QAction *action = new QAction(name.remove(LUTPath)
                                      .remove(QRegularExpression("\\.lut$")));
        LUTMenu->addAction(action);

        connect(action, &QAction::triggered, this, [ = ](){
            plot->setColorMap(new IJLUTColorMap(path));
        });
    }

    QPushButton *autoScalePushButton = new QPushButton("Autoscale");
    QSizePolicy bsp(QSizePolicy::Preferred,
                    QSizePolicy::Expanding);
    bsp.setVerticalStretch(0);
    autoScalePushButton->setSizePolicy(bsp);

    autoScalePushButton->setCheckable(true);
    autoScalePushButton->setChecked(true);

    QPushButton *LUTPushButton = new QPushButton("LUT");
    LUTPushButton->setMenu(LUTMenu);

    QHBoxLayout *controlsLayout = new QHBoxLayout();

    QVBoxLayout *vLayout;

    vLayout = new QVBoxLayout();
    vLayout->addWidget(minSlider);
    vLayout->addWidget(maxSlider);
    controlsLayout->addLayout(vLayout);

    vLayout = new QVBoxLayout();
    vLayout->addWidget(autoScalePushButton);
    vLayout->addWidget(LUTPushButton);
    controlsLayout->addLayout(vLayout);

    plot->setMinimumHeight(450);
    QWidget *aspectRatioWidget = new AspectRatioWidget(plot, 1., 1., 0, 120);
    layout->addWidget(aspectRatioWidget, 1);
    layout->addLayout(controlsLayout);

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
