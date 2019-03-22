#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

#include "core/spim.h"
#include "core/galvoramp.h"
#include "core/cameratrigger.h"

#include "nisettingswidget.h"

NISettingsWidget::NISettingsWidget(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void NISettingsWidget::setupUI()
{
    QGroupBox *natinstGroupBox = new QGroupBox("National Instruments");
    QGridLayout *grid = new QGridLayout();

    QComboBox *comboBox;

    QStringList galvoPhysChan = spim().getGalvoRamp()->getPhysicalChannels();

    int row = 0;
    for (int i = 0; i < SPIM_NCAMS; ++i) {
        row = 0;
        grid->addWidget(new QLabel(QString("Camera Trigger %1").arg(i)),
                        row++, i * 2 + 0, 1, 2);

        grid->addWidget(new QLabel("Counter"), row, i * 2 + 0);
        comboBox = new QComboBox();
        comboBox->insertItems(0, NI::getCOPhysicalChans());
        comboBox->setCurrentText(
            spim().getCameraTrigger()->getPhysicalChannel(i));
        grid->addWidget(comboBox, row++, i * 2 + 1);
        cameraTriggerCtrComboBoxList.insert(i, comboBox);

        grid->addWidget(new QLabel("Term"), row, 0);
        comboBox = new QComboBox();
        comboBox->insertItems(0, NI::getTerminals());
        comboBox->setCurrentText(spim().getCameraTrigger()->getTerm(i));
        grid->addWidget(comboBox, row++, i * 2 + 1);
        cameraTriggerTermComboBoxList.insert(i, comboBox);

        grid->addWidget(new QLabel(QString("Galvo ramp %1").arg(i)),
                        row++, i * 2 + 0, 1, 2);
        grid->addWidget(new QLabel("Channel"), row, i * 2 + 0);
        comboBox = new QComboBox();
        comboBox->addItems(NI::getAOPhysicalChans());
        comboBox->setCurrentText(galvoPhysChan.at(i));
        grid->addWidget(comboBox, row++, i * 2 + 1);
        galvoRampComboBoxList.insert(i, comboBox);
    }

    QPushButton *NIApplyPushButton = new QPushButton("Apply");
    grid->addWidget(NIApplyPushButton, row++, 0, 1, 4);

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
    QStringList ctrs;
    QStringList terms;
    QStringList galvoPhysChan;
    for (int i = 0; i < 2; ++i) {
        galvoPhysChan << galvoRampComboBoxList.at(i)->currentText();
        ctrs << cameraTriggerCtrComboBoxList.at(i)->currentText();
        terms << cameraTriggerTermComboBoxList.at(i)->currentText();
    }
    spim().getGalvoRamp()->setPhysicalChannels(galvoPhysChan);
    spim().getCameraTrigger()->setPhysicalChannels(ctrs);
    spim().getCameraTrigger()->setTerms(terms);
    spim().setupGalvoRampTriggerSource(terms);
}
