#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

#include "core/spim.h"

#include "nisettingswidget.h"

NISettingsWidget::NISettingsWidget(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void NISettingsWidget::setupUI()
{
    QGroupBox *natinstGroupBox = new QGroupBox("National Instruments");
    QGridLayout *grid = new QGridLayout();

    int row = 0;
    grid->addWidget(new QLabel("Camera Trigger"), row++, 0, 1, 2);

    grid->addWidget(new QLabel("Counter"), row, 0);
    cameraTriggerCtrComboBox = new QComboBox();
    cameraTriggerCtrComboBox->insertItems(0, NI::getCOPhysicalChans());
    grid->addWidget(cameraTriggerCtrComboBox, row++, 1);

    grid->addWidget(new QLabel("Term"), row, 0);
    cameraTriggerTermComboBox = new QComboBox();
    cameraTriggerTermComboBox->addItems(NI::getTerminals());
    grid->addWidget(cameraTriggerTermComboBox, row++, 1);

    int idx;
    idx = cameraTriggerCtrComboBox->findData(
        spim().getCameraTrigger()->getPhysicalChannel());
    cameraTriggerCtrComboBox->setCurrentIndex(idx);

    idx = cameraTriggerTermComboBox->findData(
        spim().getCameraTrigger()->getTerm());
    cameraTriggerTermComboBox->setCurrentIndex(idx);

    grid->addWidget(new QLabel("Galvo ramp"), row++, 0, 1, 2);
    grid->addWidget(new QLabel("Channel"), row, 0);
    galvoRampComboBox = new QComboBox();
    galvoRampComboBox->addItems(NI::getAOPhysicalChans());
    grid->addWidget(galvoRampComboBox, row++, 1);

    QPushButton *NIApplyPushButton = new QPushButton("Apply");
    grid->addWidget(NIApplyPushButton, row++, 0, 1, 2);

    natinstGroupBox->setLayout(grid);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(natinstGroupBox);
    layout->setMargin(0);
    setLayout(layout);

    connect(NIApplyPushButton, &QPushButton::clicked,
            this, &NISettingsWidget::apply);
}

void NISettingsWidget::apply()
{
    spim().getGalvoRamp()->setPhysicalChannel(galvoRampComboBox->currentText());
    spim().setupCameraTrigger(cameraTriggerCtrComboBox->currentText(),
                              cameraTriggerTermComboBox->currentText());
}
