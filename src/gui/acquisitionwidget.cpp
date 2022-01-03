#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QFileDialog>
#include <QCheckBox>
#include <QButtonGroup>
#include <QRadioButton>

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
    devs << spim().getStackStage();
    devs << spim().getMosaicStages();

    QGridLayout *grid = new QGridLayout();

    int row = 0;
    int col = 1;

    grid->addWidget(new QLabel("From"), row, col++);
    grid->addWidget(new QLabel("To"), row, col++);
    grid->addWidget(new QLabel("Step"), row++, col++);

    void (QDoubleSpinBox::* valueChanged)(double d) = &QDoubleSpinBox::valueChanged;


    for (const SPIM_PI_DEVICES d_enum : devs) {
        PIDevice *dev = spim().getPIDevice(d_enum);
        QList<double> *scanRange = spim().getScanRange(d_enum);
        col = 0;
        QCheckBox *checkBox;
        if (spim().getMosaicStages().contains(d_enum)) {
            checkBox = new QCheckBox(dev->getVerboseName());
            grid->addWidget(checkBox, row, col++);
        } else {
            grid->addWidget(new QLabel(dev->getVerboseName()), row, col++);
        }

        QList<QWidget *> wList;

        for (int i = 0; i < 3; ++i) {
            QDoubleSpinBox *sb = new QDoubleSpinBox();
            wList << sb;
            sb->setDecimals(SPIM_SCAN_DECIMALS);
            sb->setSuffix(" mm");
            sb->setRange(-999, 999);
            sb->setValue(scanRange->at(i));

            connect(sb, valueChanged, this, [ = ](double d){
                scanRange->replace(i, d);
            });

            connect(dev, &PIDevice::connected, this, [ = ](){
                double minVal = dev->getTravelRangeLowEnd("1").at(0);
                double maxVal = dev->getTravelRangeHighEnd("1").at(0);
                sb->setRange(minVal, maxVal);
            });

            grid->addWidget(sb, row, col++);
        }

        if (spim().getMosaicStages().contains(d_enum)) {
            for (QWidget *w : wList) {
                w->setEnabled(false);
            }
            connect(checkBox, &QCheckBox::toggled, [ = ](bool checked){
                for (QWidget *w : wList) {
                    w->setEnabled(checked);
                }
                spim().setMosaicStageEnabled(d_enum, checked);
            });
            checkBox->setChecked(spim().isMosaicStageEnabled(d_enum));
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

    QButtonGroup *binningRadioGroup = new QButtonGroup();

    QRadioButton *noBinningRadioButton  = new QRadioButton("None");
    QRadioButton *twoBinningRadioButton = new  QRadioButton("2x2");
    QRadioButton *fourBinningRadioButton = new  QRadioButton("4x4");

    QList<QRadioButton *> radios;
    radios << noBinningRadioButton << twoBinningRadioButton << fourBinningRadioButton;
    QMap<uint, QRadioButton *> radioMap;

    int i = 0;
    for (QRadioButton *radio : radios) {
        binningRadioGroup->addButton(radio);
        connect(radio, &QRadioButton::toggled, [ = ](){
            spim().setBinning(1 << i);
        });
        radioMap[1 << i] = radio;
        i++;
    }

    radioMap[spim().getBinning()]->setChecked(true);

    col = 0;
    grid->addWidget(new QLabel("Binning"), row, col++);
    grid->addWidget(noBinningRadioButton, row, col++);
    grid->addWidget(twoBinningRadioButton, row, col++);
    grid->addWidget(fourBinningRadioButton, row, col++);

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
