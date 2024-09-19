#include "cameradelaywidget.h"

#include "cameratrigger.h"
#include "spim.h"
#include "tasks.h"

#include <qtlab/widgets/customspinbox.h>

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>

CameraDelayWidget::CameraDelayWidget(QWidget *parent)
    : QWidget{parent}
{
    setupUI();
}

void CameraDelayWidget::setupUI()
{
    int row = 0;
    int col = 0;

    CameraTrigger *ct = spim().getTasks()->getCameraTrigger();

    QGridLayout *grid = new QGridLayout();
    col = 0;
    col++;
    grid->addWidget(new QLabel("Delay"), row, col++);
    row++;

    for (int i = 0; i < SPIM_NCAMS + SLAVE_SPIM_NCAMS; ++i) {
        col = 0;

        grid->addWidget(new QLabel(QString("Cam %1").arg(i)), row, col++);

        DoubleSpinBox *delaySpinbox = new DoubleSpinBox();
        delaySpinbox->setRange(0, 100);
        delaySpinbox->setDecimals(3);
        delaySpinbox->setSingleStep(0.01);
        delaySpinbox->setSuffix(" ms");
        delaySpinbox->setValue(ct->getCameraDelay(i) * 1000.);
        grid->addWidget(delaySpinbox, row, col++);

        connect(delaySpinbox, &DoubleSpinBox::returnPressed, [=]() {
            ct->setCameraDelay(i, delaySpinbox->value() / 1000.);
        });

        row++;
    }

    QGroupBox *gbox = new QGroupBox("Camera Delay");
    gbox->setLayout(grid);

    QBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(gbox);
    setLayout(layout);
}
