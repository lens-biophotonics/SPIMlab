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

galvoswidget::widgetboxes(galvoRamp *something)
{
        int row = 0;
        int col = 0;
        QVector<double> wp = something->getWaveformParams();
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
            something->setWaveformAmplitude(i, amplitudeSpinBox->value());
            something->setWaveformOffset(i, offsetSpinBox->value());
            something->setWaveformDelay(i, delaySpinBox->value() / 1000.);
            something->setWaveformRampFraction(i, fractionSpinBox->value() / 100.);
            something->updateWaveform();}
        
        connect(offsetSpinBox, &DoubleSpinBox::returnPressed, this, apply);
        connect(amplitudeSpinBox, &DoubleSpinBox::returnPressed, this, apply);
        connect(delaySpinBox, &DoubleSpinBox::returnPressed, this, apply);
        connect(fractionSpinBox, &DoubleSpinBox::returnPressed, this, apply);
}

galvosWidget::setupUI()
{
    QGridLayout *grid = new QGridLayout();
    int row = 0;
    int col = 0;
    GalvoRamp *smth;
    
    grid->addWidget(new QLabel("Offset"), row, col);
    grid->addWidget(new QLabel("Amplitude"), row, col++);
    grid->addWidget(new QLabel("Delay"), row, col++);
    grid->addWidget(new QLabel("Fraction"), row, col++);
    
    grid->addWidget(new QLabel("Fourier plane galvo pair"), row++, 0, 1, 1); 
    grid->addWidget(new QLabel("Generating Light Sheet"), row++, 0, 1, 1);
    grid->addWidget(new GalvoWaveformWidget(), row++, 0, 1, 1);  //Reusing the same class for two mirrors of G2
    
    grid->addWidget(new QLabel("Diagonal correction waveform"), row++, 0, 1, 1); //the second mirror of G2
    for (int j = 0; j<SPIM_NCAMS; j++){
        smth = spim().getCorrectionGalvo[j];  //we first point to the two mirrors of g2
        widgetboxes(smth); }

    grid->addWidget(new QLabel("Image plane galvo pair"), row++, 0, 1, 1); //now the G1 galvos
    
    grid->addWidget(new QLabel("Rapid beta 1 correction"), row++, 0, 1, 1); //the two mirrors of g1 we move thanks to with RAPID
    for (int j = 2; j<SPIM_NCAMS; j++){
        smth = spim().getCorrectionGalvo[j];  
        widgetboxes(smth); }
    
    grid->addWidget(new QLabel("Descan inclination correction"), row++, 0, 1, 1); //the two mirrors of g1 we move thanks to g3's descan inclination
    for (int j = 4; j<SPIM_NCAMS; j++){
        smth = spim().getCorrectionGalvo[j];  
        widgetboxes(smth); }    

    grid->addWidget(new QLabel("Descanning galvo "), row++, 0, 1, 1); //G3's parameters
    smth = spim().getCorrectionGalvo[6];  
    widgetboxes(smth);  

    QGroupBox *gbox = new QGroupBox("Galvo Ramp");
    gbox->setLayout(grid);

    QBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(gbox);
    setLayout(layout);
}
        

    


    QBoxLayout *LSGalvo = new QHBoxLayout();
    LSGalvo->addWidget(new GalvoWaveformWidget());
    controlsHLayout->addLayout(galvoProgressLayout);




