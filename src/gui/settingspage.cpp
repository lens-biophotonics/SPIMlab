#include <QHBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

#include "spim.h"

#include "settingspage.h"
#include "nisettingswidget.h"
#include "picontrollersettingswidget.h"
#include "settings.h"
#include "version.h"


SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void SettingsPage::setupUI()
{
    QHBoxLayout *piHLayout = new QHBoxLayout();
    QHBoxLayout *piHLayout2 = new QHBoxLayout();
    for (int i = 0; i < 3; ++i) {
        piHLayout->addWidget(
            new PIControllerSettingsWidget(spim().getPIDevice(i)));
    }
    for (int i = 3; i < 5; ++i) {
        piHLayout2->addWidget(
            new PIControllerSettingsWidget(spim().getPIDevice(i)));
    }

    piHLayout->addStretch();
    piHLayout2->addStretch();

    NISettingsWidget *nisw = new NISettingsWidget();

    QLineEdit *LUTPathLineEdit = new QLineEdit();
    LUTPathLineEdit->setReadOnly(true);
    LUTPathLineEdit->setText(
        settings().value(SETTINGSGROUP_OTHERSETTINGS, SETTING_LUTPATH)
        .toString());

    QDoubleSpinBox *scanVelocitySpinBox = new QDoubleSpinBox();
    scanVelocitySpinBox->setDecimals(4);
    scanVelocitySpinBox->setValue(spim().getScanVelocity());

    QPushButton * chooseLUTPathPushButton = new QPushButton("...");

    QGroupBox *otherSettingsGB = new QGroupBox("Other settings");
    {
        QGridLayout *grid = new QGridLayout();

        int row = 0;
        int col = 0;

        grid->addWidget(new QLabel("LUT path"), row, col++);
        grid->addWidget(LUTPathLineEdit, row, col++);
        grid->addWidget(chooseLUTPathPushButton, row++, col++);

        col = 0;
        grid->addWidget(new QLabel("Scan velocity"), row, col++);
        grid->addWidget(scanVelocitySpinBox, row++, col++);

        QVBoxLayout *vLayout = new QVBoxLayout();
        vLayout->addLayout(grid);
        vLayout->addStretch();

        otherSettingsGB->setLayout(vLayout);
    }

    connect(chooseLUTPathPushButton, &QPushButton::clicked, this, [ = ](){
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setOption(QFileDialog::ShowDirsOnly, true);

        if (!dialog.exec())
            return;

        QString path = dialog.selectedFiles().at(0);
        LUTPathLineEdit->setText(path);

        settings().setValue(SETTINGSGROUP_OTHERSETTINGS, SETTING_LUTPATH, path);

        QMessageBox::information(
            this, "Info", QString("Please restart %1").arg(PROGRAM_NAME));
    });


    void (QDoubleSpinBox::* mySignal)(double) = &QDoubleSpinBox::valueChanged;
    connect(scanVelocitySpinBox, mySignal, &spim(), &SPIM::setScanVelocity);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(nisw);
    hLayout->addWidget(otherSettingsGB);
    hLayout->addStretch();

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addLayout(piHLayout);
    vlayout->addLayout(piHLayout2);
    vlayout->addLayout(hLayout);
    vlayout->addStretch();

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addLayout(vlayout);
    hlayout->addStretch();
    setLayout(hlayout);
}
