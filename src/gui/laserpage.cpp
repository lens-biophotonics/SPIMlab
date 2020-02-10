#include <QBoxLayout>

#include "spim.h"

#include "laserpage.h"
#include "coboltwidget.h"
#include "filterwheelwidget.h"

LaserPage::LaserPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void LaserPage::setupUI()
{
    QBoxLayout *hl = new QHBoxLayout();
    QBoxLayout *hf = new QHBoxLayout();

    for (int i = 0; i < SPIM_NCOBOLT; i++) {
        hl->addWidget(new CoboltWidget(spim().getLaser(i)));
    }

    for (int i = 0; i < SPIM_NFILTERWHEEL; i++) {
        hf->addWidget(new FilterWheelWidget(spim().getFilterWheel(i), i));
    }
    hf->addStretch();

    QBoxLayout *bl = new QVBoxLayout();
    bl->addLayout(hl);
    bl->addLayout(hf);
    bl->addStretch();
    setLayout(bl);
}
