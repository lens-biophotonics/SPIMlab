#include <QBoxLayout>

#include "core/spim.h"

#include "coboltpage.h"
#include "coboltwidget.h"
#include "filterwheelwidget.h"

CoboltPage::CoboltPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void CoboltPage::setupUI()
{
    QBoxLayout *hl = new QHBoxLayout();
    QBoxLayout *hf = new QHBoxLayout();

    for (int i = 0; i < SPIM_NCOBOLT; i++) {
        hl->addWidget(new CoboltWidget(spim().getLaser(i)));
    }

    for (int i = 0; i < SPIM_NFILTERWHEEL; i++) {
        hf->addWidget(new FilterWheelWidget(spim().getFilterWheel(i)));
    }
    hf->addStretch();

    QBoxLayout *bl = new QVBoxLayout();
    bl->addLayout(hl);
    bl->addLayout(hf);
    bl->addStretch();
    setLayout(bl);
}
