#include "nisettingswidget.h"

#include "cameratrigger.h"
#include "galvoramp.h"
#include "spim.h"
#include "tasks.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

NISettingsWidget::NISettingsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void NISettingsWidget::setupUI()
{
    QGroupBox *natinstGroupBox = new QGroupBox("National Instruments");
    QGridLayout *grid = new QGridLayout();

    QComboBox *comboBox;

    CameraTrigger *cameraTrigger = spim().getTasks()->getCameraTrigger();

    QStringList terminals = NI::getTerminals().filter("PFI");

    int row = 0;
    for (int i = 0; i < SPIM_NCAMS; ++i) {
        row = 0;
        grid->addWidget(new QLabel(QString("Camera %1").arg(i)), row++, i * 2 + 0, 1, 2);

        grid->addWidget(new QLabel("Trigger"), row, i * 2 + 0);
        comboBox = new QComboBox();
        comboBox->insertItems(0, terminals);
        comboBox->setCurrentText(cameraTrigger->getPulseTerms().at(i));
        grid->addWidget(comboBox, row++, i * 2 + 1);
        cameraTriggerPulseComboBoxList.insert(i, comboBox);

        grid->addWidget(new QLabel("Blanking"), row, i * 2 + 0);
        comboBox = new QComboBox();
        comboBox->insertItems(0, terminals);
        comboBox->setCurrentText(cameraTrigger->getBlankingPulseTerms().at(i));
        grid->addWidget(comboBox, row++, i * 2 + 1);
        blankingComboBoxList.insert(i, comboBox);

        grid->addWidget(new QLabel("Galvo"), row, i * 2 + 0);
        comboBox = new QComboBox();
        comboBox->addItems(NI::getAOPhysicalChans());
        comboBox->setCurrentText(spim().getTasks()->getGalvoRamp()->getPhysicalChannels().at(i));
        grid->addWidget(comboBox, row++, i * 2 + 1);
        galvoRampComboBoxList.insert(i, comboBox);
    }

    QFrame *line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    grid->addWidget(line, row++, 0, 1, 4);

    PITriggerOutputComboBox = new QComboBox();
    PITriggerOutputComboBox->insertItems(0, terminals);
    PITriggerOutputComboBox->setCurrentText(cameraTrigger->getStartTriggerTerm());
    grid->addWidget(new QLabel("PI trig out"), row, 0);
    grid->addWidget(PITriggerOutputComboBox, row++, 1);

    QPushButton *NIApplyPushButton = new QPushButton("Apply");
    grid->addWidget(NIApplyPushButton, row++, 0, 1, 4);

    natinstGroupBox->setLayout(grid);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(natinstGroupBox);
    layout->setMargin(0);
    setLayout(layout);

    connect(NIApplyPushButton, &QPushButton::clicked, this, &NISettingsWidget::apply);
}

void NISettingsWidget::apply()
{
    QStringList pulseTerms;
    QStringList blankingTerms;
    QStringList galvoRampPhysChans;

    for (int i = 0; i < SPIM_NCAMS; ++i) {
        pulseTerms << cameraTriggerPulseComboBoxList.at(i)->currentText();
        blankingTerms << blankingComboBoxList.at(i)->currentText();
        galvoRampPhysChans << galvoRampComboBoxList.at(i)->currentText();
    }

    GalvoRamp *gr = spim().getTasks()->getGalvoRamp();
    gr->setPhysicalChannels(galvoRampPhysChans);
    CameraTrigger *ct = spim().getTasks()->getCameraTrigger();
    ct->setStartTriggerTerm(PITriggerOutputComboBox->currentText());
    ct->setPulseTerms(pulseTerms);
    ct->setBlankingPulseTerms(blankingTerms);
}
