#include "galvosWidget.h"

#include <qtlab/widgets/customspinbox.h>

#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>

galvosWidget::galvosWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void descanGalvo {


  
}

void galvosWidget::setupUi{

    QGridLayout *grid = new QGridLayout();

    int row = 0;
    grid->addWidget(new QLabel("Generating Light Sheet"), row, 0, 1, 1);
    grid->addWidget(new GalvoWaveformWidget(), row++, 0, 1, 1);
  
    grid->addWidget(new QLabel("Descanning Light Sheet"), row++, 0, 1, 1);
    grid->addWidget(new descanGalvo(), row++, 0, 1, 1);

  
  
    QBoxLayout *LSGalvo = new QHBoxLayout();
    LSGalvo->addWidget(new GalvoWaveformWidget());
    controlsHLayout->addLayout(galvoProgressLayout);

  
    for (i=0;i<2;i++){
      QWidget galvorampwidg = new QWidget;


      
      
    }
   



  
}

for (int i = 0; i < SPIM_NCAMS; ++i) {
        col = 0;
        QVector<double> wp = gr->getWaveformParams();
        wp = wp.mid(i * GALVORAMP_N_OF_PARAMS, GALVORAMP_N_OF_PARAMS);

        DoubleSpinBox *offsetSpinBox = new DoubleSpinBox();
        offsetSpinBox->setRange(-10, 10);
        offsetSpinBox->setDecimals(3);
        offsetSpinBox->setSingleStep(0.01);
        offsetSpinBox->setSuffix(" V");
        offsetSpinBox->setValue(wp.at(GALVORAMP_OFFSET_IDX));
        grid->addWidget(offsetSpinBox, row, col++);

        DoubleSpinBox *amplitudeSpinBox = new DoubleSpinBox();
        amplitudeSpinBox->setRange(-10, 10);
        amplitudeSpinBox->setDecimals(3);
        amplitudeSpinBox->setSingleStep(0.01);
        amplitudeSpinBox->setSuffix(" V");
        amplitudeSpinBox->setValue(wp.at(GALVORAMP_AMPLITUDE_IDX));
        grid->addWidget(amplitudeSpinBox, row, col++);

        DoubleSpinBox *delaySpinBox = new DoubleSpinBox();
        delaySpinBox->setRange(-1000, 1000);
        delaySpinBox->setDecimals(3);
        delaySpinBox->setSingleStep(0.05);
        delaySpinBox->setSuffix(" ms");
        delaySpinBox->setValue(wp.at(GALVORAMP_DELAY_IDX) * 1000);
        grid->addWidget(delaySpinBox, row, col++);

        DoubleSpinBox *fractionSpinBox = new DoubleSpinBox();
        fractionSpinBox->setRange(0, 100);
        fractionSpinBox->setDecimals(2);
        fractionSpinBox->setSingleStep(0.1);
        fractionSpinBox->setSuffix(" %");
        fractionSpinBox->setValue(wp.at(GALVORAMP_FRACTION_IDX) * 100);
        grid->addWidget(fractionSpinBox, row, col++);
