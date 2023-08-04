#include "nisettingswidget.h"

#include "cameratrigger.h"
#include "galvoramp.h"
#include "spim.h"
#include "tasks.h"

#include <QComboBox>
#include <QDoubleSpinBox>
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

    CameraTrigger *ct = spim().getTasks()->getCameraTrigger();

    QStringList terminals = NI::getTerminals().filter("PFI");

#ifdef DEMO_MODE
    terminals << "/DemoDev/PFI0"
              << "/DemoDev/PFI1";
#endif

    QList<QComboBox *> galvoRampComboBoxList;
    QList<QComboBox *> blankingComboBoxList;
    QList<QComboBox *> cameraTriggerPulseComboBoxList;

    QDoubleSpinBox *delaySpinBox = new QDoubleSpinBox();
    delaySpinBox->setSuffix("ms");
    delaySpinBox->setRange(0, 100);
    delaySpinBox->setDecimals(2);
    delaySpinBox->setValue(ct->getDelay());

    connect(delaySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        ct->setDelay(value);
    });

    int row = 0;
    for (int i = 0; i < SPIM_NCAMS; ++i) {
        row = 0;

        grid->addWidget(new QLabel(QString("Camera %1").arg(i)), row++, i * 2 + 0, 1, 2);
        grid->addWidget(new QLabel("Trigger"), row, i * 2 + 0);
        comboBox = new QComboBox();
        comboBox->insertItems(0, terminals);
        comboBox->setCurrentText(ct->getPulseTerms().at(i));
        grid->addWidget(comboBox, row++, i * 2 + 1);
        cameraTriggerPulseComboBoxList.insert(i, comboBox);

        if (i == 0) {
            grid->addWidget(new QLabel("Blanking"), row, i * 2 + 0);
            comboBox = new QComboBox();
            comboBox->insertItems(0, terminals);
            comboBox->setCurrentText(ct->getBlankingPulseTerms().at(i));
            grid->addWidget(comboBox, row++, i * 2 + 1);
            blankingComboBoxList.insert(i, comboBox);

            grid->addWidget(new QLabel("Illumination arms:"), row++, 0);
        } else {
            grid->addWidget(new QLabel("Delay"), row, i * 2 + 0);
            grid->addWidget(delaySpinBox, row++, i * 2 + 1);
            row += 2;
        }

        int col = 0;
        grid->addWidget(new QLabel(QString("Galvo %1").arg(i)), row, col + 0);
        comboBox = new QComboBox();
        comboBox->addItems(NI::getAOPhysicalChans());
#ifdef DEMO_MODE
        comboBox->addItems({"DemoDev/ao0", "DemoDev/ao1"});
#endif
        comboBox->setCurrentText(spim().getTasks()->getGalvoRamp()->getPhysicalChannels().at(i));
        grid->addWidget(comboBox, row++, col + 1);
        galvoRampComboBoxList.insert(i, comboBox);
    }

    QFrame *line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    grid->addWidget(line, row++, 0, 1, 4);

    QComboBox *PITriggerOutputComboBox = new QComboBox();
    PITriggerOutputComboBox->insertItems(0, terminals);
    PITriggerOutputComboBox->setCurrentText(ct->getStartTriggerTerm());
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
            if (i == 0) {
                blankingTerms << blankingComboBoxList.at(i)->currentText();
            }
            galvoRampPhysChans << galvoRampComboBoxList.at(i)->currentText();
        }

        GalvoRamp *gr = spim().getTasks()->getGalvoRamp();
        gr->setPhysicalChannels(galvoRampPhysChans);
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
}
