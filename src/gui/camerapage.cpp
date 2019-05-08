#include <QHBoxLayout>
#include <QList>
#include <QPushButton>
#include <QLabel>

#include "core/spim.h"

#include "pipositioncontrolwidget.h"
#include "acquisitionwidget.h"
#include "cameradisplay.h"
#include "camerapage.h"
#include "controlwidget.h"
#include "galvowaveformwidget.h"

CameraPage::CameraPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void CameraPage::setupUI()
{
    QHBoxLayout *cameraHLayout = new QHBoxLayout();
    for (int i = 0; i < SPIM_NCAMS; ++i) {
        QVBoxLayout *vLayout = new QVBoxLayout();
        vLayout->addWidget(new CameraDisplay(spim().getCamera(i)));
        cameraHLayout->addLayout(vLayout, 1);
    }
    cameraHLayout->addStretch(0);

    PIPositionControlWidget *cw = new PIPositionControlWidget();
    cw->setTitle("Translational stages");
    cw->appendRow(spim().getPIDevice(SPIM::PI_DEVICE_X_AXIS), "1", "X");
    cw->appendRow(spim().getPIDevice(SPIM::PI_DEVICE_Y_AXIS), "1", "Y");
    cw->appendRow(spim().getPIDevice(SPIM::PI_DEVICE_Z_AXIS), "1", "Z");
    cw->appendRow(spim().getPIDevice(SPIM::PI_DEVICE_LEFT_OBJ_AXIS), "1", "Z L");
    cw->appendRow(spim().getPIDevice(SPIM::PI_DEVICE_RIGHT_OBJ_AXIS), "1", "Z R");

    QBoxLayout *galvoProgressLayout = new QVBoxLayout();
    galvoProgressLayout->addWidget(new GalvoWaveformWidget());
    galvoProgressLayout->addStretch();

    QHBoxLayout *controlsHLayout = new QHBoxLayout();
    controlsHLayout->addWidget(cw);
    controlsHLayout->addWidget(new AcquisitionWidget());
    controlsHLayout->addLayout(galvoProgressLayout);
    controlsHLayout->addStretch();
    controlsHLayout->addWidget(new ControlWidget());

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(cameraHLayout, 1);
    vLayout->addLayout(controlsHLayout);

    setLayout(vLayout);
}
