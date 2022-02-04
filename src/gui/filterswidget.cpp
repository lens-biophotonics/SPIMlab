#include "filterswidget.h"

#include "filterwheelwidget.h"
#include "spim.h"
#include "thorlabsmcwidget.h"

#include <qtlab/hw/serial-widgets/aa_aotf_widget.h>

#include <QBoxLayout>
#include <QGroupBox>

FiltersWidget::FiltersWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void FiltersWidget::setupUI()
{
    QBoxLayout *vLeft = new QVBoxLayout();
    QBoxLayout *vRight = new QVBoxLayout();

    QGroupBox *aotfGb;
    QBoxLayout *lay;

    aotfGb = new QGroupBox("AOTF");
    lay = new QHBoxLayout();
#define AOTF_LINES 4
    lay->addWidget(new AA_AOTFWidget(spim().getAOTF(0), AOTF_LINES));
    aotfGb->setLayout(lay);

    QLabel *title;
    title = new QLabel();
    title->setText("Camera 1");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("QLabel {font-weight: bold; font-size: 14pt;}");
    vLeft->addWidget(title);
    vLeft->addWidget(new FilterWheelWidget(spim().getFilterWheel(0), 0));
    vLeft->addWidget(aotfGb);
    vLeft->addStretch();

    QGroupBox *motorControllerGb = new QGroupBox("Thorlabs Motor Controller");
    lay = new QHBoxLayout();
    lay->addWidget(new ThorlabsMCWidget(spim().getMotorController()));
    motorControllerGb->setLayout(lay);

    title = new QLabel();
    title->setText("Camera 2");

    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("QLabel {font-weight: bold; font-size: 14pt;}");
    vRight->addWidget(title);
    vRight->addWidget(new FilterWheelWidget(spim().getFilterWheel(1), 1));
    vRight->addWidget(motorControllerGb);
    vRight->addStretch();

    QBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addLayout(vLeft);
    hLayout->addLayout(vRight);

    setLayout(hLayout);
}
