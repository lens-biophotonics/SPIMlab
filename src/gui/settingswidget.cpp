#include "settingswidget.h"

#include "settings.h"
#include "spim.h"
#include "version.h"

#ifdef MASTER_SPIM
#include "nisettingswidget.h"
#endif

#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void SettingsWidget::setupUI()
{
#ifdef MASTER_SPIM
    NISettingsWidget *nisw = new NISettingsWidget();
#endif

    QLineEdit *LUTPathLineEdit = new QLineEdit();
    LUTPathLineEdit->setReadOnly(true);
    LUTPathLineEdit->setText(
        settings().value(SETTINGSGROUP_OTHERSETTINGS, SETTING_LUTPATH).toString());

#ifdef MASTER_SPIM
    QDoubleSpinBox *scanVelocitySpinBox = new QDoubleSpinBox();
    scanVelocitySpinBox->setDecimals(4);
    scanVelocitySpinBox->setValue(spim().getScanVelocity());
#endif

    QPushButton *chooseLUTPathPushButton = new QPushButton("...");

    QStringList outputPath = spim().getOutputPathList();

    QLineEdit *leftCamPathLineEdit = new QLineEdit(outputPath.at(0));
    QLineEdit *rightCamPathLineEdit = new QLineEdit(outputPath.at(1));
    leftCamPathLineEdit->setReadOnly(true);
    rightCamPathLineEdit->setReadOnly(true);
    QPushButton *chooseLeftCampPathPushButton = new QPushButton("...");
    QPushButton *chooseRightCamPathPushButton = new QPushButton("...");

    connect(chooseLeftCampPathPushButton, &QPushButton::clicked, this, [=]() {
        QFileDialog dialog;
        dialog.setDirectory(leftCamPathLineEdit->text());
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setOption(QFileDialog::ShowDirsOnly, true);

        if (!dialog.exec())
            return;

        QString path = dialog.selectedFiles().at(0);
        leftCamPathLineEdit->setText(path);
        QStringList sl = spim().getOutputPathList();
        sl[0] = path;
        spim().setOutputPathList(sl);
        settings().setValue(SETTINGSGROUP_OTHERSETTINGS, SETTING_CAM_OUTPUT_PATH_LIST, sl);
    });

    connect(chooseRightCamPathPushButton, &QPushButton::clicked, this, [=]() {
        QFileDialog dialog;
        dialog.setDirectory(rightCamPathLineEdit->text());
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setOption(QFileDialog::ShowDirsOnly, true);

        if (!dialog.exec())
            return;

        QString path = dialog.selectedFiles().at(0);
        rightCamPathLineEdit->setText(path);
        QStringList sl = spim().getOutputPathList();
        sl[1] = path;
        spim().setOutputPathList(sl);
        settings().setValue(SETTINGSGROUP_OTHERSETTINGS, SETTING_CAM_OUTPUT_PATH_LIST, sl);
    });

    QGroupBox *otherSettingsGB = new QGroupBox("Other settings");
    {
        QGridLayout *grid = new QGridLayout();

        int row = 0;
        int col = 0;

        grid->addWidget(new QLabel("LUT path"), row, col++);
        grid->addWidget(LUTPathLineEdit, row, col++);
        grid->addWidget(chooseLUTPathPushButton, row++, col++);

#ifdef MASTER_SPIM
        col = 0;
        grid->addWidget(new QLabel("Scan velocity"), row, col++);
        grid->addWidget(scanVelocitySpinBox, row++, col++);
#endif

        col = 0;
        grid->addWidget(new QLabel("Left camera path"), row, col++);
        grid->addWidget(leftCamPathLineEdit, row, col++);
        grid->addWidget(chooseLeftCampPathPushButton, row++, col++);

        col = 0;
        grid->addWidget(new QLabel("Right camera path"), row, col++);
        grid->addWidget(rightCamPathLineEdit, row, col++);
        grid->addWidget(chooseRightCamPathPushButton, row++, col++);

        QVBoxLayout *vLayout = new QVBoxLayout();
        vLayout->addLayout(grid);
        vLayout->addStretch();

        otherSettingsGB->setLayout(vLayout);
    }

    connect(chooseLUTPathPushButton, &QPushButton::clicked, this, [=]() {
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setOption(QFileDialog::ShowDirsOnly, true);

        if (!dialog.exec())
            return;

        QString path = dialog.selectedFiles().at(0);
        LUTPathLineEdit->setText(path);

        settings().setValue(SETTINGSGROUP_OTHERSETTINGS, SETTING_LUTPATH, path);

        QMessageBox::information(this, "Info", QString("Please restart %1").arg(PROGRAM_NAME));
    });

#ifdef MASTER_SPIM
    void (QDoubleSpinBox::*mySignal)(double) = &QDoubleSpinBox::valueChanged;
    connect(scanVelocitySpinBox, mySignal, &spim(), &SPIM::setScanVelocity);
#endif

    QHBoxLayout *hLayout = new QHBoxLayout();
#ifdef MASTER_SPIM
    hLayout->addWidget(nisw);
#endif
    hLayout->addWidget(otherSettingsGB);
    hLayout->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(hLayout);
    vLayout->addStretch(1);

    setLayout(vLayout);
}
