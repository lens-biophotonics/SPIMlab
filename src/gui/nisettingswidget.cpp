#include "nisettingswidget.h"

#include "cameratrigger.h"
#include "galvoramp.h"
#include "spim.h"
#include "tasks.h"

#include <QCheckBox>
#include <QComboBox>
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

    QCheckBox *checkBox;

    CameraTrigger *cameraTrigger = spim().getTasks()->getCameraTrigger();

    QStringList terminals = NI::getTerminals().filter("PFI");

#ifdef DEMO_MODE
    terminals << "/DemoDev/PFI0"
              << "/DemoDev/PFI1";
#endif

    QList<QComboBox *> galvoRampComboBoxList;
    QList<QComboBox *> blankingComboBoxList;
    QList<QComboBox *> cameraTriggerPulseComboBoxList;
    QList<QCheckBox *> checkBoxes;

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

#ifdef DEMO_MODE
        comboBox->addItems({"DemoDev/ao0", "DemoDev/ao1"});
#endif
        comboBox->setCurrentText(spim().getTasks()->getGalvoRamp()->getPhysicalChannels().at(i));
        grid->addWidget(comboBox, row++, i * 2 + 1);
        galvoRampComboBoxList.insert(i, comboBox);

        grid->addWidget(new QLabel("Enabled"), row, i * 2 + 0);
        checkBox = new QCheckBox();
        grid->addWidget(checkBox, row++, i * 2 + 1);
        checkBoxes.insert(i, checkBox);
    }

    QFrame *line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    grid->addWidget(line, row++, 0, 1, 4);

    QComboBox *PITriggerOutputComboBox = new QComboBox();
    PITriggerOutputComboBox->insertItems(0, terminals);
    PITriggerOutputComboBox->setCurrentText(cameraTrigger->getStartTriggerTerm());
    grid->addWidget(new QLabel("PI trig out"), row, 0);
    grid->addWidget(PITriggerOutputComboBox, row++, 1);

    natinstGroupBox->setLayout(grid);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(natinstGroupBox);
    layout->setMargin(0);
    setLayout(layout);

    auto apply = [=]() {
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
    };

    QList<QComboBox *> allCombos;
    allCombos << galvoRampComboBoxList << blankingComboBoxList << cameraTriggerPulseComboBoxList
              << PITriggerOutputComboBox;
    for (QComboBox *combo : allCombos) {
        connect(combo, QOverload<int>::of(&QComboBox::activated), [=]() { apply(); });
    }

    int i = 0;
    for (QCheckBox *checkBox : checkBoxes) {
        connect(checkBox, &QCheckBox::toggled, [=](bool checked) {
            spim().setCameraEnabled(i, checkBox->QCheckBox::isChecked());
        });
        checkBox->setChecked(spim().isCameraEnabled(i));
        i++;
    }
}
