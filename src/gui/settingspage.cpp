#include <QHBoxLayout>

#include "core/spim.h"

#include "settingspage.h"
#include "nisettingswidget.h"
#include "picontrollersettingswidget.h"


SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void SettingsPage::setupUI()
{
    QHBoxLayout *piHLayout = new QHBoxLayout();
    QHBoxLayout *piHLayout2 = new QHBoxLayout();
    piHLayout->addWidget(
        new PIControllerSettingsWidget(
            spim().piDevice(SPIM::PI_DEVICE_X_AXIS)));
    piHLayout->addWidget(
        new PIControllerSettingsWidget(
            spim().piDevice(SPIM::PI_DEVICE_Y_AXIS)));
    piHLayout->addWidget(
        new PIControllerSettingsWidget(
            spim().piDevice(SPIM::PI_DEVICE_Z_AXIS)));
    piHLayout->addStretch();

    piHLayout2->addWidget(
        new PIControllerSettingsWidget(
            spim().piDevice(SPIM::PI_DEVICE_LEFT_OBJ_AXIS)));
    piHLayout2->addWidget(
        new PIControllerSettingsWidget(
            spim().piDevice(SPIM::PI_DEVICE_RIGHT_OBJ_AXIS)));
    piHLayout2->addStretch();

    NISettingsWidget *nisw = new NISettingsWidget();

    QHBoxLayout *niHLayout = new QHBoxLayout();
    niHLayout->addWidget(nisw);
    niHLayout->addStretch();

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addLayout(piHLayout);
    vlayout->addLayout(piHLayout2);
    vlayout->addLayout(niHLayout);
    vlayout->addStretch();

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addLayout(vlayout);
    hlayout->addStretch();
    setLayout(hlayout);
}
