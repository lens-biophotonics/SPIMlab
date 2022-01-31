#include <QHBoxLayout>

#include <qtlab/hw/pi-widgets/picontrollersettingswidget.h>

#include "spim.h"
#include "stagewidget.h"

StageWidget::StageWidget(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *piHLayout = new QHBoxLayout();
    QHBoxLayout *piHLayout2 = new QHBoxLayout();
    for (int i = 0; i < 3; ++i) {
        piHLayout->addWidget(
            new PIControllerSettingsWidget(spim().getPIDevice(i)));
    }
    for (int i = 3; i < 5; ++i) {
        piHLayout2->addWidget(
            new PIControllerSettingsWidget(spim().getPIDevice(i)));
    }

    piHLayout->addStretch();
    piHLayout2->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(piHLayout);
    vLayout->addLayout(piHLayout2);
    vLayout->addStretch();
    setLayout(vLayout);
}
