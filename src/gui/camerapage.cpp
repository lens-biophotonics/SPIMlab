#include <QHBoxLayout>
#include <QList>
#include <QPushButton>
#include <QLabel>

#include <qtlab/widgets/cameradisplay.h>
#include <qtlab/widgets/cameraplot.h>
#include <qtlab/widgets/customspinbox.h>

#include "spim.h"
#include "settings.h"

#include "acquisitionwidget.h"
#include "camerapage.h"
#include "controlwidget.h"
#include "galvowaveformwidget.h"
#include "progresswidget.h"
#include "displayworker.h"

CameraPage::CameraPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

CameraPage::~CameraPage()
{
    saveSettings();
}

void CameraPage::setupUI()
{
    const Settings s = settings();
    QString LUTPath = s.value(SETTINGSGROUP_OTHERSETTINGS, SETTING_LUTPATH).toString();
    QHBoxLayout *cameraHLayout = new QHBoxLayout();
    for (int i = 0; i < SPIM_NCAMS; ++i) {
        QVBoxLayout *vLayout = new QVBoxLayout();
        CameraDisplay *cd = new CameraDisplay();
        cd->setTitle(QString("Cam %1").arg(i));
        cd->setPlotSize(QSize(2048, 2048));
        DisplayWorker *worker = new DisplayWorker(spim().getCamera(i), cd);
        void (CameraPlot::*fp)(const double*, const size_t) = &CameraPlot::setData;
        connect(worker, &DisplayWorker::newImage, cd->getPlot(), fp);
        cd->setLUTPath(LUTPath);
        vLayout->addWidget(cd);
        cameraHLayout->addLayout(vLayout, 1);
    }
    cameraHLayout->addStretch(0);

    cw = new PIPositionControlWidget();
    cw->setTitle("Translational stages");
    cw->appendRow(spim().getPIDevice(PI_DEVICE_X_AXIS), "1", "X");
    cw->appendRow(spim().getPIDevice(PI_DEVICE_Y_AXIS), "1", "Y");
    cw->appendRow(spim().getPIDevice(PI_DEVICE_Z_AXIS), "1", "Z");
    cw->appendRow(spim().getPIDevice(PI_DEVICE_LEFT_OBJ_AXIS), "1", "Z L");
    cw->appendRow(spim().getPIDevice(PI_DEVICE_RIGHT_OBJ_AXIS), "1", "Z R");

    for (int i = 0; i < SPIM_NPIDEVICES; ++i) {
        QString g = SETTINGSGROUP_AXIS(i);
        cw->getPositionSpinbox(i)->setValue(s.value(g, SETTING_POS).toDouble());
        cw->getVelocitySpinBox(i)->setValue(s.value(g, SETTING_VELOCITY).toDouble());
        cw->getStepSpinBox(i)->setValue(s.value(g, SETTING_STEPSIZE).toDouble());
    }

    QBoxLayout *stageLayout = new QVBoxLayout();
    stageLayout->addWidget(cw);

    QBoxLayout *acquisitionLayout = new QVBoxLayout();
    acquisitionLayout->addWidget(new AcquisitionWidget());

    QBoxLayout *galvoProgressLayout = new QVBoxLayout();
    galvoProgressLayout->addWidget(new GalvoWaveformWidget());
    galvoProgressLayout->addWidget(new ProgressWidget());

    QHBoxLayout *controlsHLayout = new QHBoxLayout();
    controlsHLayout->addLayout(stageLayout);
    controlsHLayout->addLayout(acquisitionLayout);
    controlsHLayout->addLayout(galvoProgressLayout);
    controlsHLayout->addStretch();
    controlsHLayout->addWidget(new ControlWidget());

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(cameraHLayout, 1);
    vLayout->addLayout(controlsHLayout);

    setLayout(vLayout);
}

void CameraPage::saveSettings()
{
    Settings s = settings();
    for (int i = 0; i < SPIM_NPIDEVICES; ++i) {
        QString g = SETTINGSGROUP_AXIS(i);
        s.setValue(g, SETTING_POS, cw->getPositionSpinbox(i)->value());
        s.setValue(g, SETTING_VELOCITY, cw->getVelocitySpinBox(i)->value());
        s.setValue(g, SETTING_STEPSIZE, cw->getStepSpinBox(i)->value());
    }
}
