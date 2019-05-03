#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>

#include "core/statemachine.h"
#include "core/spim.h"
#include "core/pidevice.h"
#include "acquisitionwidget.h"

AcquisitionWidget::AcquisitionWidget(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void AcquisitionWidget::setupUI()
{
    setEnabled(false);
    stateMachine().getState(STATE_CAPTURING)->assignProperty(
        this, "enabled", false);
    stateMachine().getState(STATE_READY)->assignProperty(
        this, "enabled", true);

    QList<SPIM::PI_DEVICES> devs;
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

    for (const SPIM::PI_DEVICES d_enum : devs) {
        PIDevice *dev = spim().getPIDevice(d_enum);
        QList<double> *scanRange = spim().getScanRange(d_enum);
        col = 0;
        grid->addWidget(new QLabel(dev->getVerboseName()), row, col++);

        for (int i = 0; i < 3; ++i) {
            QDoubleSpinBox *sb = new QDoubleSpinBox();
            sb->setDecimals(4);
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

    col = 0;
    grid->addWidget(new QLabel("Path"), row, col++);
    QLineEdit *lineEdit = new QLineEdit();
    lineEdit->setText(spim().getOutputPath());
    grid->addWidget(lineEdit, row, col, 1, 2); col += 2;
    QPushButton *pushButton = new QPushButton("...");
    grid->addWidget(pushButton, row, col++);

    QBoxLayout *boxLayout;

    boxLayout = new QVBoxLayout();
    QGroupBox *gb = new QGroupBox("Acquisition");
    boxLayout->addLayout(grid);
    gb->setLayout(boxLayout);

    boxLayout = new QVBoxLayout(this);
    boxLayout->addWidget(gb);
    setLayout(boxLayout);

    connect(lineEdit, &QLineEdit::textChanged, &spim(), &SPIM::setOutputPath);

    connect(pushButton, &QPushButton::clicked, this, [ = ](){
        QFileDialog dialog;
        dialog.setDirectory(lineEdit->text());
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setOption(QFileDialog::ShowDirsOnly, true);

        if (!dialog.exec())
            return;

        QString path = dialog.selectedFiles().at(0);
        lineEdit->setText(path);
    });
}
