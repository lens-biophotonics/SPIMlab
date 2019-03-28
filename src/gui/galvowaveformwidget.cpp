#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

#include "core/spim.h"
#include "core/galvoramp.h"
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
    QLabel *label;

    offsetSpinBox = new QDoubleSpinBox();
    offsetSpinBox->setRange(-10, 10);
    offsetSpinBox->setSuffix(" V");
    offsetSpinBox->setValue(wp.at(GALVORAMP_OFFSET_IDX));
    label = new QLabel("Offset");
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hLayout->addWidget(label);
    hLayout->addWidget(offsetSpinBox);

    amplitudeSpinBox = new QDoubleSpinBox();
    amplitudeSpinBox->setRange(-10, 10);
    amplitudeSpinBox->setSuffix(" V");
    amplitudeSpinBox->setValue(wp.at(GALVORAMP_AMPLITUDE_IDX));
    label = new QLabel("Amplitude");
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hLayout->addWidget(label);
    hLayout->addWidget(amplitudeSpinBox);

    delaySpinBox = new QDoubleSpinBox();
    delaySpinBox->setSuffix(" ms");
    delaySpinBox->setRange(-1000, 1000);
    delaySpinBox->setValue(wp.at(GALVORAMP_DELAY_IDX) * 1000);
    label = new QLabel("Delay");
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hLayout->addWidget(label);
    hLayout->addWidget(delaySpinBox);

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
    gr->setWaveformDelay(chNumber, delaySpinBox->value() / 1000.);
}
