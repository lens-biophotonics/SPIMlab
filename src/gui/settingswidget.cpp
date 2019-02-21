#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>

#include "core/spim.h"

#include "settingswidget.h"
#include "picontrollersettingswidget.h"

SettingsWidget::SettingsWidget(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void SettingsWidget::setupUI()
{
    QGroupBox *natinstGroupBox = new QGroupBox("National Instruments");
    QGridLayout *grid = new QGridLayout();

    grid->addWidget(new QLabel("Camera Trigger"), 0, 0, 1, 2);

    grid->addWidget(new QLabel("Counter"), 1, 0);
    QComboBox *cameraTriggerCtrComboBox = new QComboBox();
    cameraTriggerCtrComboBox->insertItems(0, NI::getCOPhysicalChans());
    grid->addWidget(cameraTriggerCtrComboBox, 1, 1);

    grid->addWidget(new QLabel("Term"), 2, 0);
    QComboBox *cameraTriggerTermComboBox = new QComboBox();
    cameraTriggerTermComboBox->addItems(NI::getTerminals());
    grid->addWidget(cameraTriggerTermComboBox, 2, 1);

    natinstGroupBox->setLayout(grid);

    QHBoxLayout *piHLayout = new QHBoxLayout();
    QHBoxLayout *piHLayout2 = new QHBoxLayout();
    piHLayout->addWidget(
        new PIControllerSettingsWidget(
            "X Axis", spim().piDevice(SPIM::PI_DEVICE_X_AXIS)));
    piHLayout->addWidget(
        new PIControllerSettingsWidget(
            "Y Axis", spim().piDevice(SPIM::PI_DEVICE_Y_AXIS)));
    piHLayout->addWidget(
        new PIControllerSettingsWidget(
            "Z Axis", spim().piDevice(SPIM::PI_DEVICE_Z_AXIS)));
    piHLayout->addStretch();

    piHLayout2->addWidget(
        new PIControllerSettingsWidget(
            "Left Objective", spim().piDevice(SPIM::PI_DEVICE_LEFT_OBJ_AXIS)));
    piHLayout2->addWidget(
        new PIControllerSettingsWidget(
            "Right Objective",
            spim().piDevice(SPIM::PI_DEVICE_RIGHT_OBJ_AXIS)));
    piHLayout2->addStretch();

    QHBoxLayout *niHLayout = new QHBoxLayout();
    niHLayout->addWidget(natinstGroupBox);
    niHLayout->addStretch();

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addLayout(piHLayout);
    vlayout->addLayout(piHLayout2);
    vlayout->addLayout(niHLayout);
    vlayout->addStretch();

    QHBoxLayout *hlayout = new QHBoxLayout(this);
    hlayout->addLayout(vlayout);
    hlayout->addStretch();
    setLayout(hlayout);
}
