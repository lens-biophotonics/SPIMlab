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

    QList<QVariant> wp = spim().getGalvoRamp()->getWaveformParams();

    offsetSpinBox = new QDoubleSpinBox();
    offsetSpinBox->setRange(-10, 10);
    offsetSpinBox->setSuffix(" V");
    offsetSpinBox->setValue(wp.at(0).toDouble());
    hLayout->addWidget(new QLabel("Offset"));
    hLayout->addWidget(offsetSpinBox);

    amplitudeSpinBox = new QDoubleSpinBox();
    amplitudeSpinBox->setRange(-10, 10);
    amplitudeSpinBox->setSuffix(" V");
    amplitudeSpinBox->setValue(wp.at(1).toDouble());
    hLayout->addWidget(new QLabel("Amplitude"));
    hLayout->addWidget(amplitudeSpinBox);

    delaySpinBox = new QSpinBox();
    delaySpinBox->setRange(-1000, 1000);
    delaySpinBox->setSuffix(" samples");
    delaySpinBox->setValue(wp.at(2).toInt());
    hLayout->addWidget(new QLabel("Delay"));
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
    spim().getGalvoRamp()->setWaveformParams(
        offsetSpinBox->value(), amplitudeSpinBox->value(),
        delaySpinBox->value());
}
