#include <QHBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

#include "core/spim.h"

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
    piHLayout->addWidget(
        new PIControllerSettingsWidget(
            spim().piDevice(SPIM::PI_DEVICE_X_AXIS)));
    piHLayout->addWidget(
        new PIControllerSettingsWidget(
            spim().piDevice(SPIM::PI_DEVICE_Y_AXIS)));
    piHLayout->addWidget(
        new PIControllerSettingsWidget(
            spim().piDevice(SPIM::PI_DEVICE_Z_AXIS)));
    piHLayout->addStretch();

    piHLayout2->addWidget(
        new PIControllerSettingsWidget(
            spim().piDevice(SPIM::PI_DEVICE_LEFT_OBJ_AXIS)));
    piHLayout2->addWidget(
        new PIControllerSettingsWidget(
            spim().piDevice(SPIM::PI_DEVICE_RIGHT_OBJ_AXIS)));
    piHLayout2->addStretch();

    NISettingsWidget *nisw = new NISettingsWidget();

    QLineEdit *LUTPathLineEdit = new QLineEdit();
    LUTPathLineEdit->setReadOnly(true);
    LUTPathLineEdit->setText(
        settings().value(SETTINGSGROUP_OTHERSETTINGS, SETTING_LUTPATH)
        .toString());

    QPushButton * chooseLUTPathPushButton = new QPushButton("...");

    QGroupBox *otherSettingsGB = new QGroupBox("Other settings");
    {
        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->addWidget(new QLabel("LUT path"));
        hLayout->addWidget(LUTPathLineEdit);
        hLayout->addWidget(chooseLUTPathPushButton);

        QVBoxLayout *vLayout = new QVBoxLayout();
        vLayout->addLayout(hLayout);
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
