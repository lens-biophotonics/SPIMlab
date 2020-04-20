#include <QHBoxLayout>
#include <QList>
#include <QPushButton>
#include <QLabel>

#include <qtlab/widgets/cameradisplay.h>

#include "spim.h"
#include "settings.h"

#include "pipositioncontrolwidget.h"
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

void CameraPage::setupUI()
{
    QString LUTPath = settings().value(SETTINGSGROUP_OTHERSETTINGS,
                                       SETTING_LUTPATH).toString();
    QHBoxLayout *cameraHLayout = new QHBoxLayout();
    for (int i = 0; i < SPIM_NCAMS; ++i) {
        QVBoxLayout *vLayout = new QVBoxLayout();
        CameraDisplay *cd = new CameraDisplay();
        cd->setTitle(QString("Cam %1").arg(i));
        DisplayWorker *worker = new DisplayWorker(spim().getCamera(i),
                                                  cd->getBuffer());
        connect(worker, &DisplayWorker::newImage, cd, &CameraDisplay::replot);
        cd->setLUTPath(LUTPath);
        vLayout->addWidget(cd);
        cameraHLayout->addLayout(vLayout, 1);
    }
    cameraHLayout->addStretch(0);

    PIPositionControlWidget *cw = new PIPositionControlWidget();
    cw->setTitle("Translational stages");
    cw->appendRow(PI_DEVICE_X_AXIS, "1", "X");
    cw->appendRow(PI_DEVICE_Y_AXIS, "1", "Y");
    cw->appendRow(PI_DEVICE_Z_AXIS, "1", "Z");
    cw->appendRow(PI_DEVICE_LEFT_OBJ_AXIS, "1", "Z L");
    cw->appendRow(PI_DEVICE_RIGHT_OBJ_AXIS, "1", "Z R");

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
