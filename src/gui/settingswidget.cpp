#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>

#include "settingswidget.h"
#include "core/natinst.h"

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

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addWidget(natinstGroupBox);
    vlayout->addStretch();

    QHBoxLayout *hlayout = new QHBoxLayout(this);
    hlayout->addLayout(vlayout);
    hlayout->addStretch();
    setLayout(hlayout);
}
