#include <QBoxLayout>

#include "core/spim.h"

#include "coboltpage.h"
#include "coboltwidget.h"

CoboltPage::CoboltPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void CoboltPage::setupUI()
{
    QBoxLayout *hl = new QHBoxLayout();

    for (int i = 0; i < SPIM_NCOBOLT; i++) {
        hl->addWidget(new CoboltWidget(spim().getLaser(i)));
    }

    QBoxLayout *bl = new QVBoxLayout();
    bl->addLayout(hl);
    bl->addStretch();
    setLayout(bl);
}
