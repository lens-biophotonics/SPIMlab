#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

#include "spim.h"
#include "galvoramp.h"
#include "cameratrigger.h"

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

        grid->addWidget(new QLabel("Line"), row, i * 2 + 0);
        comboBox = new QComboBox();
        comboBox->insertItems(0, NI::getDOLines());
        comboBox->setCurrentText(
            spim().getCameraTrigger()->getPhysicalChannel(i * 2));
        grid->addWidget(comboBox, row++, i * 2 + 1);
        cameraTriggerCtrComboBoxList.insert(i, comboBox);

        grid->addWidget(new QLabel("Blanking"), row, i * 2 + 0);
        comboBox = new QComboBox();
        comboBox->insertItems(0, NI::getDOLines());
        comboBox->setCurrentText(
            spim().getCameraTrigger()->getPhysicalChannel(i * 2 + 1));
        grid->addWidget(comboBox, row++, i * 2 + 1);
        aotfBlankingComboBoxList.insert(i, comboBox);

        grid->addWidget(new QLabel(QString("Galvo ramp %1").arg(i)),
                        row++, i * 2 + 0, 1, 2);
        grid->addWidget(new QLabel("Channel"), row, i * 2 + 0);
        comboBox = new QComboBox();
        comboBox->addItems(NI::getAOPhysicalChans());
        comboBox->setCurrentText(galvoPhysChan.at(i));
        grid->addWidget(comboBox, row++, i * 2 + 1);
        galvoRampComboBoxList.insert(i, comboBox);
    }

    galvoRampTrigger = new QComboBox();
    galvoRampTrigger->insertItems(0, NI::getTerminals());
    galvoRampTrigger->setCurrentText(spim().getGalvoRamp()->getTriggerTerm());
    grid->addWidget(new QLabel("Galvo ramp trigger"), row, 0, 1, 2);
    grid->addWidget(galvoRampTrigger, row++, 2, 1, 2);

    PITriggerOutput = new QComboBox();
    PITriggerOutput->insertItems(0, NI::getTerminals());
    PITriggerOutput->setCurrentText(spim().getCameraTrigger()->getTriggerTerm());
    grid->addWidget(new QLabel("PI trigger output"), row, 0, 1, 2);
    grid->addWidget(PITriggerOutput, row++, 2, 1, 2);

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
    QStringList galvoPhysChan;
    for (int i = 0; i < 2; ++i) {
        galvoPhysChan << galvoRampComboBoxList.at(i)->currentText();
        ctrs << cameraTriggerCtrComboBoxList.at(i)->currentText();
        ctrs << aotfBlankingComboBoxList.at(i)->currentText();
    }
    GalvoRamp *gr = spim().getGalvoRamp();
    gr->setPhysicalChannels(galvoPhysChan);
    gr->setTriggerTerm(galvoRampTrigger->currentText());
    CameraTrigger *ct = spim().getCameraTrigger();
    ct->setPhysicalChannels(ctrs);
    ct->setTriggerTerm(PITriggerOutput->currentText());
}
