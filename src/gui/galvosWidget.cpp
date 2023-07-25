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

galvosWidget::setupUI()
{
    QGridLayout *grid = new QGridLayout();
    int row = 0;
    grid->addWidget(new QLabel("Fourier plane galvo pair"), row, 0, 1, 1);
    grid->addWidget(new QLabel("Generating Light Sheet"), row++, 0, 1, 1);
    grid->addWidget(new GalvoWaveformWidget(), row++, 0, 1, 1);  //x2

    list of strings titles= ["Diagonal correction", "Rapid correction", "Inclination correction"];
    for (int j=0;j<size(titles) ;j++){
        grid->addWidget(new QLabel(titles[j]), row++, 0, 1, 1);
    
        GalvoRamp *smth = new GalvoRamp;
        smth->spim().correctionGalvos[j];
        
        //GalvoRamp *gr = spim().getTasks()->getGalvoRamp();
    
        int col = 0;
        grid->addWidget(new QLabel("Offset"), row++, col++);
        grid->addWidget(new QLabel("Amplitude"), row, col++);
        grid->addWidget(new QLabel("Delay"), row, col++);
        grid->addWidget(new QLabel("Fraction"), row, col++);
        row++;
            for (int i = 0; i < SPIM_NCAMS; ++i) {
                col = 0;
                //QVector<double> wp = gr->getWaveformParams();
                QVector<double> wp = smth->getWaveformParams();
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

                std::function<void(void)> apply = [=]() {
                    smth->setWaveformAmplitude(i, amplitudeSpinBox->value());
                    smth->setWaveformOffset(i, offsetSpinBox->value());
                    smth->setWaveformDelay(i, delaySpinBox->value() / 1000.);
                    smth->setWaveformRampFraction(i, fractionSpinBox->value() / 100.);
                    smth->updateWaveform();
                /*
                gr->setWaveformAmplitude(i, amplitudeSpinBox->value());
                gr->setWaveformOffset(i, offsetSpinBox->value());
                gr->setWaveformDelay(i, delaySpinBox->value() / 1000.);
                gr->setWaveformRampFraction(i, fractionSpinBox->value() / 100.);
                gr->updateWaveform();
                */
                };

                connect(offsetSpinBox, &DoubleSpinBox::returnPressed, this, apply);
                connect(amplitudeSpinBox, &DoubleSpinBox::returnPressed, this, apply);
                connect(delaySpinBox, &DoubleSpinBox::returnPressed, this, apply);
                connect(fractionSpinBox, &DoubleSpinBox::returnPressed, this, apply);
                
                delete smth;
                row++;
            }
    }
}
        
/*
    list of strings main_titles = ["Fourier plane galvo pair","Image plane galvo pair","Descan galvo"];
    QGroupBox *gbox = new QGroupBox(main_titles[h]);
    gbox->setLayout(grid);

    QBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(gbox);
    setLayout(layout);
*/
    


    QBoxLayout *LSGalvo = new QHBoxLayout();
    LSGalvo->addWidget(new GalvoWaveformWidget());
    controlsHLayout->addLayout(galvoProgressLayout);




