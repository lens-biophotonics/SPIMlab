#include <QBoxLayout>
#include <QGroupBox>

#include "spim.h"

#include "laserpage.h"
#include "coboltwidget.h"
#include "filterwheelwidget.h"

#include "aotfwidget.h"

LaserPage::LaserPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void LaserPage::setupUI()
{
    QBoxLayout *hl = new QHBoxLayout();

    for (int i = 0; i < SPIM_NCOBOLT; i++) {
        hl->addWidget(new CoboltWidget(spim().getLaser(i)));
    }

    QBoxLayout *vLeft = new QVBoxLayout();
    QBoxLayout *vRight = new QVBoxLayout();

    QGroupBox *aotfGb;
    QBoxLayout *lay;

    aotfGb = new QGroupBox("AOTF");
    lay = new QHBoxLayout();
    lay->addWidget(new AOTFWidget(spim().getAOTF(0)));
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
    lay->addWidget(new AOTFWidget(spim().getAOTF(1)));
    aotfGb->setLayout(lay);

    title = new QLabel("Right view");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("QLabel {font-weight: bold; font-size: 14pt;}");
    vRight->addWidget(title);
    vRight->addWidget(new FilterWheelWidget(spim().getFilterWheel(1), 1));
    vRight->addWidget(aotfGb);
    vRight->addStretch();

    QBoxLayout *hf = new QHBoxLayout();
    hf->addLayout(vLeft);
    hf->addLayout(vRight);
    hf->addStretch();

    QBoxLayout *bl = new QVBoxLayout();
    bl->addLayout(hl);
    bl->addLayout(hf);
    bl->addStretch();

    setLayout(bl);
}
