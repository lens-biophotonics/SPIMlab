#include <QHBoxLayout>
#include <QList>
#include <QPushButton>
#include <QLabel>

#include "core/spim.h"
#include "core/statemachine.h"

#include "galvowaveformwidget.h"
#include "pipositioncontrolwidget.h"
#include "cameradisplay.h"
#include "camerapage.h"

CameraPage::CameraPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void CameraPage::setupUI()
{
    QList<QWidget *> wList;  // list of widgets to be disabled during capturing
    wList.reserve(6);

    QList<SPIM::PI_DEVICES> piList = {
        SPIM::PI_DEVICE_LEFT_OBJ_AXIS,
        SPIM::PI_DEVICE_RIGHT_OBJ_AXIS,
    };
    QHBoxLayout *cameraHLayout = new QHBoxLayout();
    for (int i = 0; i < SPIM_NCAMS; ++i) {
        QVBoxLayout *vLayout = new QVBoxLayout();
        vLayout->addWidget(new CameraDisplay(spim().getCamera(i)));
        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->addWidget(new GalvoWaveformWidget(i));
        hLayout->addStretch();

        auto *picw = new PIPositionControlWidget();
        picw->setTitle("Focus");
        picw->appendRow(spim().piDevice(piList.at(i)), "1", "Objective");

        QHBoxLayout *piHLayout = new QHBoxLayout();
        piHLayout->addWidget(picw);
        piHLayout->addStretch();

        vLayout->addLayout(hLayout);
        vLayout->addLayout(piHLayout);
        cameraHLayout->addLayout(vLayout);
    }
    cameraHLayout->addStretch();

    QDoubleSpinBox *expTimeSpinBox = new QDoubleSpinBox();
    expTimeSpinBox->setRange(0, 10000);
    expTimeSpinBox->setDecimals(3);
    expTimeSpinBox->setSuffix(" ms");
    expTimeSpinBox->setValue(spim().getExposureTime());
    QPushButton *setExpTimePushButton = new QPushButton("Set");
    wList.append(setExpTimePushButton);

    QHBoxLayout *controlsHLayout0 = new QHBoxLayout();
    controlsHLayout0->addWidget(new QLabel("Exposure Time"));
    controlsHLayout0->addWidget(expTimeSpinBox);
    controlsHLayout0->addWidget(setExpTimePushButton);
    controlsHLayout0->addStretch();

    PIPositionControlWidget *cw = new PIPositionControlWidget();
    cw->setTitle("Sample translation stages");
    cw->appendRow(spim().piDevice(SPIM::PI_DEVICE_X_AXIS), "1", "X");
    cw->appendRow(spim().piDevice(SPIM::PI_DEVICE_Y_AXIS), "1", "Y");
    cw->appendRow(spim().piDevice(SPIM::PI_DEVICE_Z_AXIS), "1", "Z");

    QHBoxLayout *controlsHLayout1 = new QHBoxLayout();
    controlsHLayout1->addWidget(cw);
    controlsHLayout1->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(cameraHLayout);
    vLayout->addLayout(controlsHLayout0);
    vLayout->addLayout(controlsHLayout1);
    vLayout->addStretch();

    setLayout(vLayout);

    QState *readyState = stateMachine().getState(STATE_READY);
    QState *capturingState = stateMachine().getState(STATE_CAPTURING);
    QState *uninitState = stateMachine().getState(STATE_UNINITIALIZED);
    foreach(auto w, wList) {
        readyState->assignProperty(w, "enabled", true);
        capturingState->assignProperty(w, "enabled", false);
        uninitState->assignProperty(w, "enabled", false);
    }

    connect(setExpTimePushButton, &QPushButton::clicked, [ = ](){
        spim().setExposureTime(expTimeSpinBox->value());
    });
}
