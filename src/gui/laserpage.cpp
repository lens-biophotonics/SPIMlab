#include <QBoxLayout>
#include <QGroupBox>

#include "spim.h"

#include "laserpage.h"
#include "coboltwidget.h"
#include "filterwheelwidget.h"

#include <qtlab/hw/serial-widgets/aa_aotf_widget.h>

LaserPage::LaserPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void LaserPage::setupUI()
{
    QBoxLayout *laserLayout = new QVBoxLayout();

    for (int i = 0; i < SPIM_NCOBOLT; i++) {
        laserLayout->addWidget(new CoboltWidget(spim().getLaser(i)));
    }
    laserLayout->addStretch();

    QBoxLayout *vLeft = new QVBoxLayout();
    QBoxLayout *vRight = new QVBoxLayout();

    QGroupBox *aotfGb;
    QBoxLayout *lay;

    aotfGb = new QGroupBox("AOTF");
    lay = new QHBoxLayout();
    lay->addWidget(new AA_AOTFWidget(spim().getAOTF(0)));
    aotfGb->setLayout(lay);

    QLabel *title;
    title = new QLabel("Left view");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("QLabel {font-weight: bold; font-size: 14pt;}");
    vLeft->addWidget(title);
    vLeft->addWidget(new FilterWheelWidget(spim().getFilterWheel(0), 0));
    vLeft->addWidget(aotfGb);
    vLeft->addStretch();

    aotfGb = new QGroupBox("AOTF");
    lay = new QHBoxLayout();
    lay->addWidget(new AA_AOTFWidget(spim().getAOTF(1)));
    aotfGb->setLayout(lay);

    title = new QLabel("Right view");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("QLabel {font-weight: bold; font-size: 14pt;}");
    vRight->addWidget(title);
    vRight->addWidget(new FilterWheelWidget(spim().getFilterWheel(1), 1));
    vRight->addWidget(aotfGb);
    vRight->addStretch();

    QBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addLayout(vLeft);
    hLayout->addLayout(vRight);
    hLayout->addStretch();
    hLayout->addLayout(laserLayout);

    setLayout(hLayout);
}
