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
        auto *picw = new PIPositionControlWidget();
        picw->setTitle("Focus");
        picw->appendRow(spim().getPIDevice(piList.at(i)), "1", "Objective");

        QHBoxLayout *piHLayout = new QHBoxLayout();
        piHLayout->addWidget(picw);
        piHLayout->addStretch();

        QVBoxLayout *vLayout = new QVBoxLayout();
        vLayout->addWidget(new CameraDisplay(spim().getCamera(i)));
        vLayout->addWidget(new GalvoWaveformWidget(i));
        vLayout->addLayout(piHLayout);
        cameraHLayout->addLayout(vLayout, 1);
    }
    cameraHLayout->addStretch(0);

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
    cw->appendRow(spim().getPIDevice(0), "1", "X");
    cw->appendRow(spim().getPIDevice(1), "1", "Y");
    cw->appendRow(spim().getPIDevice(2), "1", "Z");

    QHBoxLayout *controlsHLayout1 = new QHBoxLayout();
    controlsHLayout1->addWidget(cw);
    controlsHLayout1->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(cameraHLayout);
    vLayout->addLayout(controlsHLayout0);
    vLayout->addLayout(controlsHLayout1);

    setLayout(vLayout);

    QState *readyState = stateMachine().getState(STATE_READY);
    QState *capturingState = stateMachine().getState(STATE_CAPTURING);
    QState *uninitState = stateMachine().getState(STATE_UNINITIALIZED);
    for (auto w : wList) {
        readyState->assignProperty(w, "enabled", true);
        capturingState->assignProperty(w, "enabled", false);
        uninitState->assignProperty(w, "enabled", false);
    }

    connect(setExpTimePushButton, &QPushButton::clicked, [ = ](){
        spim().setExposureTime(expTimeSpinBox->value());
    });
}
