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
#ifdef DUALSPIM
#define AOTF_LINES 8
#else
#define AOTF_LINES 4
#endif
    lay->addWidget(new AA_AOTFWidget(spim().getAOTF(0), AOTF_LINES));
    aotfGb->setLayout(lay);

    QLabel *title;
    title = new QLabel();
#ifdef DUALSPIM
    title->setText("Left view");
#else
    title->setText("Camera 1");
#endif
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("QLabel {font-weight: bold; font-size: 14pt;}");
    vLeft->addWidget(title);
    vLeft->addWidget(new FilterWheelWidget(spim().getFilterWheel(0), 0));
    vLeft->addWidget(aotfGb);
    vLeft->addStretch();

#ifdef DUALSPIM
    aotfGb = new QGroupBox("AOTF");
    lay = new QHBoxLayout();
    lay->addWidget(new AA_AOTFWidget(spim().getAOTF(1)));
    aotfGb->setLayout(lay);
#else
    QGroupBox *motorControllerGb = new QGroupBox("Thorlabs Motor Controller");
    lay = new QHBoxLayout();
    lay->addWidget(new ThorlabsMCWidget(spim().getMotorController()));
    motorControllerGb->setLayout(lay);
#endif

    title = new QLabel();
#ifdef DUALSPIM
    title->setText("Right view");
#else
    title->setText("Camera 2");
#endif

    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("QLabel {font-weight: bold; font-size: 14pt;}");
    vRight->addWidget(title);
    vRight->addWidget(new FilterWheelWidget(spim().getFilterWheel(1), 1));
#ifdef DUALSPIM
    vRight->addWidget(aotfGb);
#else
    vRight->addWidget(motorControllerGb);
#endif
    vRight->addStretch();

    QBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addLayout(vLeft);
    hLayout->addLayout(vRight);

    setLayout(hLayout);
}
