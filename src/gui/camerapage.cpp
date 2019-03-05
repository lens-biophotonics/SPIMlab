#include <QHBoxLayout>

#include "core/spim.h"

#include "galvowaveformwidget.h"
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

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(cameraHLayout);
    vLayout->addStretch();

    setLayout(vLayout);
}
