#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>

#include "acquisitionwidget.h"
#include "settings.h"

AcquisitionWidget::AcquisitionWidget(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void AcquisitionWidget::setupUI()
{
    QStringList sl;
    sl << "X" << "Y";

    QGridLayout *grid = new QGridLayout();

    int row = 0;
    int col = 1;

    grid->addWidget(new QLabel("From"), row, col++);
    grid->addWidget(new QLabel("To"), row, col++);
    grid->addWidget(new QLabel("Step"), row++, col++);

    for (const QString &axis : sl) {
        col = 0;
        grid->addWidget(new QLabel(axis), row, col++);

        grid->addWidget(new QDoubleSpinBox(), row, col++);
        grid->addWidget(new QDoubleSpinBox(), row, col++);
        grid->addWidget(new QDoubleSpinBox(), row, col++);

        row++;
    }

    col = 0;
    grid->addWidget(new QLabel("Path"), row, col++);
    QLineEdit *lineEdit = new QLineEdit();
    lineEdit->setText(
        settings().value(SETTINGSGROUP_SPIM, SETTING_OUTPUTPATH).toString());
    grid->addWidget(lineEdit, row, col, 1, 2); col += 2;
    QPushButton *pushButton = new QPushButton("...");
    grid->addWidget(pushButton, row, col++);

    QBoxLayout *boxLayout;

    boxLayout = new QVBoxLayout();
    QGroupBox *gb = new QGroupBox("Acquisition");
    boxLayout->addLayout(grid);
    boxLayout->addStretch();
    gb->setLayout(boxLayout);

    boxLayout = new QVBoxLayout(this);
    boxLayout->addWidget(gb);
    setLayout(boxLayout);

    connect(pushButton, &QPushButton::clicked, this, [ = ](){
        QFileDialog dialog;
        dialog.setDirectory(lineEdit->text());
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setOption(QFileDialog::ShowDirsOnly, true);

        if (!dialog.exec())
            return;

        QString path = dialog.selectedFiles().at(0);
        settings().setValue(SETTINGSGROUP_SPIM, SETTING_OUTPUTPATH, path);
        lineEdit->setText(path);
    });
}
