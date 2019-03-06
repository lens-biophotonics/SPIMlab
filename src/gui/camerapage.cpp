#include <QHBoxLayout>
#include <QList>
#include <QPushButton>

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

    QHBoxLayout *cameraHLayout = new QHBoxLayout();
    for (int i = 0; i < 2; ++i) {
        QVBoxLayout *vLayout = new QVBoxLayout();
        vLayout->addWidget(new CameraDisplay(spim().getCamera(i)));
        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->addWidget(new GalvoWaveformWidget(spim().getGalvoRamp(i)));
        hLayout->addStretch();
        vLayout->addLayout(hLayout);
        cameraHLayout->addLayout(vLayout);
    }

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

    QHBoxLayout *controlsHLayout1 = new QHBoxLayout();
    foreach(SPIM::PI_DEVICES PIDEV, QList<SPIM::PI_DEVICES>(
                {SPIM::PI_DEVICE_X_AXIS,
                 SPIM::PI_DEVICE_Y_AXIS,
                 SPIM::PI_DEVICE_Z_AXIS})) {
        auto cw = new PIPositionControlWidget(spim().piDevice(PIDEV));
        wList.append(cw);
        controlsHLayout1->addWidget(cw);
    }
    controlsHLayout1->addStretch();

    QHBoxLayout *controlsHLayout2 = new QHBoxLayout();
    foreach(SPIM::PI_DEVICES PIDEV, QList<SPIM::PI_DEVICES>(
                {SPIM::PI_DEVICE_LEFT_OBJ_AXIS,
                 SPIM::PI_DEVICE_RIGHT_OBJ_AXIS})) {
        auto cw = new PIPositionControlWidget(spim().piDevice(PIDEV));
        wList.append(cw);
        controlsHLayout2->addWidget(cw);
    }
    controlsHLayout2->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(cameraHLayout);
    vLayout->addLayout(controlsHLayout0);
    vLayout->addLayout(controlsHLayout1);
    vLayout->addLayout(controlsHLayout2);
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
