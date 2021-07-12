#include <QHBoxLayout>
#include <QList>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

#include <qtlab/widgets/cameradisplay.h>
#include <qtlab/widgets/cameraplot.h>
#include <qtlab/widgets/customspinbox.h>

#include "spim.h"
#include "settings.h"

#include "acquisitionwidget.h"
#include "camerapage.h"
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
    QHBoxLayout *cameraHLayout = new QHBoxLayout();
    for (int i = 0; i < SPIM_NCAMS; ++i) {
        const Settings s = settings();
        QString LUTPath = s.value(SETTINGSGROUP_OTHERSETTINGS, SETTING_LUTPATH).toString();
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

    stageCw = new PIPositionControlWidget();
    stageCw->setTitle("Translational stages");
    stageCw->appendRow(spim().getPIDevice(PI_DEVICE_X_AXIS), "1", "X");
    stageCw->appendRow(spim().getPIDevice(PI_DEVICE_Y_AXIS), "1", "Y");
    stageCw->appendRow(spim().getPIDevice(PI_DEVICE_Z_AXIS), "1", "Z");
    stageCw->appendRow(spim().getPIDevice(PI_DEVICE_LEFT_OBJ_AXIS), "1", "Z L");
    stageCw->appendRow(spim().getPIDevice(PI_DEVICE_RIGHT_OBJ_AXIS), "1", "Z R");

    for (int i = 0; i < SPIM_NPIDEVICES; ++i) {
        const Settings s = settings();
        QString g = SETTINGSGROUP_AXIS(i);
        stageCw->getPositionSpinbox(i)->setValue(s.value(g, SETTING_POS).toDouble());
        stageCw->getVelocitySpinBox(i)->setValue(s.value(g, SETTING_VELOCITY).toDouble());
        stageCw->getStepSpinBox(i)->setValue(s.value(g, SETTING_STEPSIZE).toDouble());
    }

    AcquisitionWidget *acqWidget = new AcquisitionWidget();

    QBoxLayout *galvoProgressLayout = new QVBoxLayout();
    galvoProgressLayout->addWidget(new GalvoWaveformWidget());
    galvoProgressLayout->addWidget(new ProgressWidget());

    QPushButton *initPushButton = new QPushButton("Initialize");
    connect(initPushButton, &QPushButton::clicked, &spim(), [ = ](){
        initPushButton->setEnabled(false);
        spim().initialize();
    });

    QPushButton *startFreeRunPushButton = new QPushButton("Start free run");
    connect(startFreeRunPushButton, &QPushButton::clicked,
            &spim(), &SPIM::startFreeRun);

    QPushButton *startAcqPushButton = new QPushButton("Start acquisition");
    connect(startAcqPushButton, &QPushButton::clicked, [ = ](){
        if (acqWidget->getRunName().isEmpty()) {
            QMessageBox::critical(this, "Error", "Please specify a run name");
            return;
        }
        QList<QDir> outputDirs;
        for (int i = 0; i < SPIM_NCAMS; ++i) {
            outputDirs << spim().getFullOutputDir(i);
        }
        bool exists = false;
        for (QDir d : outputDirs) {
            if (d.exists()) {
                exists = true;
                break;
            }
        }
        if (exists)
        {
            QMessageBox msgBox;
            QString msg("A measurement with this run name (%1) already exists.");
            msg = msg.arg(acqWidget->getRunName());
            msgBox.setText(msg);
            msgBox.setInformativeText("Do you want to overwrite existing files?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            int ret = msgBox.exec();
            if (ret == QMessageBox::No) {
                return;
            }
        }

        QMetaObject::invokeMethod(&spim(), &SPIM::startAcquisition, Qt::QueuedConnection);
    });

    QPushButton *stopCapturePushButton = new QPushButton("Stop capture");
    connect(stopCapturePushButton, &QPushButton::clicked,
            &spim(), &SPIM::stop);

    QPushButton *emergencyStopPushButton = new QPushButton("EMERGENCY STOP");
    emergencyStopPushButton->setStyleSheet("QPushButton {color: red;}");
    connect(emergencyStopPushButton, &QPushButton::clicked,
            &spim(), &SPIM::emergencyStop);

    QLabel *statusLabel = new QLabel();
    statusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QState *s;

    s = spim().getState(SPIM::STATE_UNINITIALIZED);
    s->assignProperty(initPushButton, "enabled", true);
    s->assignProperty(startFreeRunPushButton, "enabled", false);
    s->assignProperty(startAcqPushButton, "enabled", false);
    s->assignProperty(stopCapturePushButton, "enabled", false);
    s->assignProperty(emergencyStopPushButton, "enabled", false);
    s->assignProperty(statusLabel, "text", "Uninitialized");

    s = spim().getState(SPIM::STATE_READY);
    s->assignProperty(initPushButton, "enabled", false);
    s->assignProperty(startFreeRunPushButton, "enabled", true);
    s->assignProperty(startAcqPushButton, "enabled", true);
    s->assignProperty(stopCapturePushButton, "enabled", false);
    s->assignProperty(emergencyStopPushButton, "enabled", true);
    s->assignProperty(statusLabel, "text", "Ready");

    s = spim().getState(SPIM::STATE_CAPTURING);
    s->assignProperty(startFreeRunPushButton, "enabled", false);
    s->assignProperty(startAcqPushButton, "enabled", false);
    s->assignProperty(stopCapturePushButton, "enabled", true);
    s->assignProperty(emergencyStopPushButton, "enabled", true);
    s->assignProperty(statusLabel, "text", "Capturing");

    spim().getState(SPIM::STATE_PRECAPTURE)->assignProperty(
        statusLabel, "text", "Precapture");
    spim().getState(SPIM::STATE_CAPTURE)->assignProperty(
        statusLabel, "text", "Capture");
    spim().getState(SPIM::STATE_FREERUN)->assignProperty(
        statusLabel, "text", "Free run");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(initPushButton);
    layout->addWidget(startFreeRunPushButton);
    layout->addWidget(startAcqPushButton);
    layout->addWidget(stopCapturePushButton);
    layout->addStretch();
    layout->addWidget(emergencyStopPushButton);
    layout->addWidget(statusLabel);

    QGroupBox *controlsGb = new QGroupBox("Controls");
    controlsGb->setLayout(layout);

    QHBoxLayout *controlsHLayout = new QHBoxLayout();
    controlsHLayout->addWidget(stageCw);
    controlsHLayout->addWidget(acqWidget);
    controlsHLayout->addLayout(galvoProgressLayout);
    controlsHLayout->addStretch();
    controlsHLayout->addWidget(controlsGb);

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
        s.setValue(g, SETTING_POS, stageCw->getPositionSpinbox(i)->value());
        s.setValue(g, SETTING_VELOCITY, stageCw->getVelocitySpinBox(i)->value());
        s.setValue(g, SETTING_STEPSIZE, stageCw->getStepSpinBox(i)->value());
    }
}
