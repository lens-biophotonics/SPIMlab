#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QFileDialog>

#include "spim.h"
#include <qtlab/hw/pi/pidevice.h>
#include "acquisitionwidget.h"

AcquisitionWidget::AcquisitionWidget(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

QString AcquisitionWidget::getRunName()
{
    return runNameLineEdit->text();
}

void AcquisitionWidget::setupUI()
{
    setEnabled(false);
    spim().getState(SPIM::STATE_CAPTURING)->assignProperty(
        this, "enabled", false);
    spim().getState(SPIM::STATE_READY)->assignProperty(
        this, "enabled", true);

    QList<SPIM_PI_DEVICES> devs;
    devs << spim().getMosaicStages();
    devs << spim().getStackStage();
    std::sort(devs.begin(), devs.end());

    QGridLayout *grid = new QGridLayout();

    int row = 0;
    int col = 1;

    grid->addWidget(new QLabel("From"), row, col++);
    grid->addWidget(new QLabel("To"), row, col++);
    grid->addWidget(new QLabel("Step"), row++, col++);

    void (QDoubleSpinBox::* mySignal)(double d) = &QDoubleSpinBox::valueChanged;

    for (const SPIM_PI_DEVICES d_enum : devs) {
        PIDevice *dev = spim().getPIDevice(d_enum);
        QList<double> *scanRange = spim().getScanRange(d_enum);
        col = 0;
        grid->addWidget(new QLabel(dev->getVerboseName()), row, col++);

        for (int i = 0; i < 3; ++i) {
            QDoubleSpinBox *sb = new QDoubleSpinBox();
            sb->setDecimals(SPIM_SCAN_DECIMALS);
            sb->setSuffix(" mm");
            sb->setRange(-999, 999);
            sb->setValue(scanRange->at(i));

            connect(sb, mySignal, this, [ = ](double d){
                scanRange->replace(i, d);
            });


            connect(dev, &PIDevice::connected, this, [ = ](){
                double minVal = dev->getTravelRangeLowEnd("1").at(0);
                double maxVal = dev->getTravelRangeHighEnd("1").at(0);
                sb->setRange(minVal, maxVal);
            });

            grid->addWidget(sb, row, col++);
        }

        row++;
    }

    QFrame *line;
    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    grid->addWidget(line, row++, 0, 1, 4);

    col = 0;
    grid->addWidget(new QLabel("Run name"), row, col++);
    runNameLineEdit = new QLineEdit();
    QRegularExpression rx = QRegularExpression("^\\w+[\\w.-]*");
    runNameLineEdit->setValidator(new QRegularExpressionValidator(rx));
    runNameLineEdit->setText(spim().getRunName());
    grid->addWidget(runNameLineEdit, row++, col, 1, 3); col += 3;

    QDoubleSpinBox *expTimeSpinBox = new QDoubleSpinBox();
    expTimeSpinBox->setRange(0, 10000);
    expTimeSpinBox->setDecimals(3);
    expTimeSpinBox->setSuffix(" ms");
    expTimeSpinBox->setValue(spim().getExposureTime());
    QPushButton *setExpTimePushButton = new QPushButton("Set");
    col = 0;
    grid->addWidget(new QLabel("Exposure Time"), row, col++);
    grid->addWidget(expTimeSpinBox, row, col++);
    grid->addWidget(setExpTimePushButton, row++, col++);

    QBoxLayout *boxLayout;

    boxLayout = new QVBoxLayout();
    boxLayout->addLayout(grid);
    boxLayout->addStretch();

    QGroupBox *gb = new QGroupBox("Acquisition");
    gb->setLayout(boxLayout);

    boxLayout = new QVBoxLayout(this);
    boxLayout->addWidget(gb);
    setLayout(boxLayout);

    connect(runNameLineEdit, &QLineEdit::textChanged, &spim(), &SPIM::setRunName);

    connect(setExpTimePushButton, &QPushButton::clicked, [ = ](){
        spim().setExposureTime(expTimeSpinBox->value());
    });
}
