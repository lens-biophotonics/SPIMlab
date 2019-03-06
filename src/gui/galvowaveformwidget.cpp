#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

#include "galvowaveformwidget.h"

GalvoWaveformWidget::GalvoWaveformWidget(GalvoRamp *galvoRamp, QWidget *parent)
    : QWidget(parent), galvoRamp(galvoRamp)
{
    setupUI();
}

void GalvoWaveformWidget::setupUI()
{
    QGridLayout *grid = new QGridLayout();
    int row = 0;

    QList<QVariant> wp = galvoRamp->getWaveformParams();

    offsetSpinBox = new QDoubleSpinBox();
    offsetSpinBox->setRange(-10, 10);
    offsetSpinBox->setSuffix(" V");
    offsetSpinBox->setValue(wp.at(0).toDouble());
    grid->addWidget(new QLabel("Offset"), row, 0, 1, 1);
    grid->addWidget(offsetSpinBox, row++, 1, 1, 1);

    amplitudeSpinBox = new QDoubleSpinBox();
    amplitudeSpinBox->setRange(-10, 10);
    amplitudeSpinBox->setSuffix(" V");
    amplitudeSpinBox->setValue(wp.at(1).toDouble());
    grid->addWidget(new QLabel("Amplitude"), row, 0, 1, 1);
    grid->addWidget(amplitudeSpinBox, row++, 1, 1, 1);

    delaySpinBox = new QSpinBox();
    delaySpinBox->setRange(-1000, 1000);
    delaySpinBox->setSuffix(" samples");
    delaySpinBox->setValue(wp.at(2).toInt());
    grid->addWidget(new QLabel("Delay"), row, 0, 1, 1);
    grid->addWidget(delaySpinBox, row++, 1, 1, 1);

    QPushButton *applyPushButton = new QPushButton("Apply");
    grid->addWidget(applyPushButton, row++, 0, 1, 2);

    QGroupBox *gbox = new QGroupBox("Galvo Ramp");
    gbox->setLayout(grid);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(gbox);
    setLayout(layout);

    connect(applyPushButton, &QPushButton::clicked,
            this, &GalvoWaveformWidget::apply);
}

void GalvoWaveformWidget::apply()
{
    galvoRamp->setWaveformParams(
        offsetSpinBox->value(), amplitudeSpinBox->value(),
        delaySpinBox->value());
}
