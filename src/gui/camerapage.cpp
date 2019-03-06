#include <QHBoxLayout>
#include <QList>

#include "core/spim.h"

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

    QHBoxLayout *controlsHLayout1 = new QHBoxLayout();
    foreach(SPIM::PI_DEVICES PIDEV, QList<SPIM::PI_DEVICES>(
                {SPIM::PI_DEVICE_X_AXIS,
                 SPIM::PI_DEVICE_Y_AXIS,
                 SPIM::PI_DEVICE_Z_AXIS})) {
        controlsHLayout1->addWidget(new PIPositionControlWidget(
                                        spim().piDevice(PIDEV)));
    }
    controlsHLayout1->addStretch();

    QHBoxLayout *controlsHLayout2 = new QHBoxLayout();
    foreach(SPIM::PI_DEVICES PIDEV, QList<SPIM::PI_DEVICES>(
                {SPIM::PI_DEVICE_LEFT_OBJ_AXIS,
                 SPIM::PI_DEVICE_RIGHT_OBJ_AXIS})) {
        controlsHLayout2->addWidget(new PIPositionControlWidget(
                                        spim().piDevice(PIDEV)));
    }
    controlsHLayout2->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(cameraHLayout);
    vLayout->addLayout(controlsHLayout1);
    vLayout->addLayout(controlsHLayout2);
    vLayout->addStretch();

    setLayout(vLayout);
}
