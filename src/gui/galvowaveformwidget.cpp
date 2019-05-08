#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

#include "core/spim.h"
#include "core/galvoramp.h"
#include "galvowaveformwidget.h"

GalvoWaveformWidget::GalvoWaveformWidget(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void GalvoWaveformWidget::setupUI()
{
    int row = 0;
    int col = 0;

    GalvoRamp *gr = spim().getGalvoRamp();

    QGridLayout *grid = new QGridLayout();
    col = 0;
    grid->addWidget(new QLabel("Offset"), row, col++);
    grid->addWidget(new QLabel("Amplitude"), row, col++);
    grid->addWidget(new QLabel("Delay"), row++, col++);

    for (int i = 0; i < SPIM_NCAMS; ++i) {
        col = 0;
        QVector<double> wp = spim().getGalvoRamp()->getWaveformParams();
        wp = wp.mid(i * GALVORAMP_N_OF_PARAMS, GALVORAMP_N_OF_PARAMS);

        QDoubleSpinBox *offsetSpinBox = new QDoubleSpinBox();
        offsetSpinBox->setRange(-10, 10);
        offsetSpinBox->setSuffix(" V");
        offsetSpinBox->setValue(wp.at(GALVORAMP_OFFSET_IDX));
        grid->addWidget(offsetSpinBox, row, col++);

        QDoubleSpinBox *amplitudeSpinBox = new QDoubleSpinBox();
        amplitudeSpinBox->setRange(-10, 10);
        amplitudeSpinBox->setSuffix(" V");
        amplitudeSpinBox->setValue(wp.at(GALVORAMP_AMPLITUDE_IDX));
        grid->addWidget(amplitudeSpinBox, row, col++);

        QDoubleSpinBox *delaySpinBox = new QDoubleSpinBox();
        delaySpinBox->setSuffix(" ms");
        delaySpinBox->setRange(-1000, 1000);
        delaySpinBox->setValue(wp.at(GALVORAMP_DELAY_IDX) * 1000);
        grid->addWidget(delaySpinBox, row, col++);

        QPushButton *applyPushButton = new QPushButton("Apply");
        grid->addWidget(applyPushButton, row++, col++);

        connect(applyPushButton, &QPushButton::clicked, [ = ](){
            gr->setWaveformAmplitude(i, amplitudeSpinBox->value());
            gr->setWaveformOffset(i, offsetSpinBox->value());
            gr->setWaveformDelay(i, delaySpinBox->value() / 1000.);
        });
    }


    QGroupBox *gbox = new QGroupBox("Galvo Ramp");
    gbox->setLayout(grid);

    QBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(gbox);
    setLayout(layout);
}
