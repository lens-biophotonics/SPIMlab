#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

#include "core/spim.h"
#include "galvowaveformwidget.h"

GalvoWaveformWidget::GalvoWaveformWidget(int channelNumber, QWidget *parent)
    : QWidget(parent), chNumber(channelNumber)
{
    setupUI();
}

void GalvoWaveformWidget::setupUI()
{
    QHBoxLayout *hLayout = new QHBoxLayout();

    QVector<double> wp = spim().getGalvoRamp()->getWaveformParams();
    wp = wp.mid(chNumber * GALVORAMP_N_OF_PARAMS, GALVORAMP_N_OF_PARAMS);

    offsetSpinBox = new QDoubleSpinBox();
    offsetSpinBox->setRange(-10, 10);
    offsetSpinBox->setSuffix(" V");
    offsetSpinBox->setValue(wp.at(GALVORAMP_OFFSET_IDX));
    hLayout->addWidget(new QLabel("Offset"));
    hLayout->addWidget(offsetSpinBox);

    amplitudeSpinBox = new QDoubleSpinBox();
    amplitudeSpinBox->setRange(-10, 10);
    amplitudeSpinBox->setSuffix(" V");
    amplitudeSpinBox->setValue(wp.at(GALVORAMP_AMPLITUDE_IDX));
    hLayout->addWidget(new QLabel("Amplitude"));
    hLayout->addWidget(amplitudeSpinBox);

    phaseSpinBox = new QDoubleSpinBox();
    phaseSpinBox->setSuffix(" %");
    phaseSpinBox->setRange(-100, 100);
    phaseSpinBox->setValue(wp.at(GALVORAMP_PHASE_IDX) * 100);
    hLayout->addWidget(new QLabel("Phase"));
    hLayout->addWidget(phaseSpinBox);

    QPushButton *applyPushButton = new QPushButton("Apply");
    hLayout->addWidget(applyPushButton);

    QGroupBox *gbox = new QGroupBox("Galvo Ramp");
    gbox->setLayout(hLayout);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(gbox);
    setLayout(layout);

    connect(applyPushButton, &QPushButton::clicked,
            this, &GalvoWaveformWidget::apply);
}

void GalvoWaveformWidget::apply()
{
    GalvoRamp *gr = spim().getGalvoRamp();
    gr->setWaveformAmplitude(chNumber, amplitudeSpinBox->value());
    gr->setWaveformOffset(chNumber, offsetSpinBox->value());
    gr->setWaveformPhase(chNumber, phaseSpinBox->value() / 100.);
}
