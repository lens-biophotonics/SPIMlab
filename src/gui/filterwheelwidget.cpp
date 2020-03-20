#include <QGroupBox>
#include <QGridLayout>
#include <QListView>
#include <QSerialPortInfo>
#include <QPushButton>
#include <QState>
#include <QMessageBox>
#include <QTimer>
#include <QListWidget>
#include <QStringListModel>
#include <QDialogButtonBox>

#include <qtlab/hw/serial/serialport.h>
#include <qtlab/hw/serial/filterwheel.h>
#include <qtlab/core/logger.h>

#include "filterwheelwidget.h"
#include "customspinbox.h"
#include "utils.h"
#include "settings.h"

static Logger *logger = getLogger("SerialPort");

FilterWheelWidget::FilterWheelWidget(FilterWheel *fw, int idx, QWidget *parent) :
    QWidget(parent), fw(fw), idx(idx)
{
    setupUI();

    QTimer *refreshTimer = new QTimer();
    refreshTimer->setInterval(2000);
    connect(refreshTimer, &QTimer::timeout, this, &FilterWheelWidget::refreshValues);

    connect(fw, &FilterWheel::connected, [ = ](){
        refreshTimer->start();
    });
    connect(fw, &FilterWheel::disconnected, [ = ](){
        refreshTimer->stop();
    });
}

void FilterWheelWidget::setupUI()
{
    QString group = SETTINGSGROUP_FILTERWHEEL(idx);
    filterList =  settings().value(group, SETTING_FILTER_LIST).toStringList();

    QGridLayout *grid = new QGridLayout();

    serialPortComboBox = new QComboBox();
    QListView *view = new QListView();
    view->setFixedWidth(350);
    serialPortComboBox->setView(view);
    serialPortComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    serialPortComboBox->setMinimumContentsLength(29);

    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        if (!info.manufacturer().startsWith("Thorlabs")) {
            continue;
        }
        QString descr = QString("%1 (%2, %3)")
                        .arg(info.portName())
                        .arg(info.description())
                        .arg(info.serialNumber());
        serialPortComboBox->addItem(descr, info.portName());
    }

    serialPortComboBox->setCurrentIndex(
        serialPortComboBox->findData(fw->getSerialPort()->portName()));

    serialPortComboBox->model()->sort(0);

    int row = 0;
    grid->addWidget(new QLabel("Serial port"), row, 0, 1, 1);
    grid->addWidget(serialPortComboBox, row++, 1, 1, 1);

    QHBoxLayout *hLayout = new QHBoxLayout();

    QPushButton *connectPushButton = new QPushButton("Connect");
    QPushButton *disconnectPushButton = new QPushButton("Disconnect");
    hLayout->addWidget(connectPushButton);
    hLayout->addWidget(disconnectPushButton);
    grid->addLayout(hLayout, row++, 1, 1, 1);

    filterLabel = new QLabel("Filter:");
    filterComboBox = new QComboBox();
    QListView *viewFilter = new QListView();
    viewFilter->setFixedWidth(350);
    filterComboBox->setView(viewFilter);
    filterComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    filterComboBox->setMinimumContentsLength(15);

    QPushButton *setFilterNamesPushButton = new QPushButton("Set Filter Names...");

    grid->addWidget(filterLabel, row, 0, 1, 1);
    grid->addWidget(filterComboBox, row++, 1, 1, 1);
    grid->addWidget(setFilterNamesPushButton, row++, 1, 1, 1);

    QGroupBox *gb = new QGroupBox("Filter wheel");
    gb->setLayout(grid);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addWidget(gb);

    setLayout(vlayout);

    connect(connectPushButton, &QPushButton::clicked,
            this, &FilterWheelWidget::connectDevice);
    connect(disconnectPushButton, &QPushButton::clicked,
            this, &FilterWheelWidget::disconnectDevice);

    connect(fw, &FilterWheel::connected, this, [ = ](){
        filterComboBox->addItems(filterList);
        try {
            filterComboBox->setCurrentIndex(fw->getPosition() - 1);
            logger->info(QString("Current filter: %1").arg(filterComboBox->currentText()));
            motionEnabled = true;
        }
        catch (std::runtime_error) {
        }
        refreshValues();
    });

    connect(filterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [ = ](){
        try {
            if (filterComboBox->currentIndex() >= 0 && motionEnabled) {
                int posnew = filterComboBox->currentIndex() + 1;
                fw->setPosition(posnew);
                logger->info(QString("Selected filter: %1").arg(filterComboBox->currentText()));
            }
        }
        catch (std::runtime_error e) {
            QMessageBox::critical(this, "Runtime error", e.what());
        }
    });

    QState *cs = fw->getSerialPort()->getConnectedState();
    QState *ds = fw->getSerialPort()->getDisconnectedState();

    QList<QWidget *> wList;

    // enabled when connected, disabled when disconnected
    wList = {
        disconnectPushButton,
        filterComboBox,
        filterLabel,
    };

    for (QWidget * w : wList) {
        cs->assignProperty(w, "enabled", true);
        ds->assignProperty(w, "enabled", false);
    }

    // enabled when disconnected, disabled when connected
    wList = {
        connectPushButton,
        serialPortComboBox,
    };

    for (QWidget * w : wList) {
        cs->assignProperty(w, "enabled", false);
        ds->assignProperty(w, "enabled", true);
    }

    connect(setFilterNamesPushButton, &QPushButton::clicked, [ = ](){
        QDialog *dialog = new QDialog();
        dialog->setModal(true);
        dialog->setWindowTitle("Set filter names");
        dialog->deleteLater();

        QDialogButtonBox *buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        connect(buttonBox, &QDialogButtonBox::accepted,
                dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected,
                dialog, &QDialog::reject);

        QListWidget *listWidget = new QListWidget();
        listWidget->setDragDropMode(QAbstractItemView::DragDrop);
        listWidget->setDefaultDropAction(Qt::MoveAction);
        listWidget->insertItems(0, filterList);

        for (int i = 0; i < listWidget->count(); ++i)
        {
            QListWidgetItem *item = listWidget->item(i);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }

        QVBoxLayout *vLayout = new QVBoxLayout();
        vLayout->addWidget(listWidget);
        vLayout->addWidget(buttonBox);

        dialog->setLayout(vLayout);

        dialog->setFixedSize(dialog->size());

        if (dialog->exec() != QDialog::Accepted) {
            return;
        }

        filterList.clear();
        for (int i = 0; i < listWidget->count(); ++i)
        {
            filterList << listWidget->item(i)->text();
        }

        filterComboBox->clear();
        filterComboBox->addItems(filterList);
        settings().setValue(group, SETTING_FILTER_LIST, filterList);
    });
}

void FilterWheelWidget::connectDevice()
{
    try {
        fw->getSerialPort()->setPortName(
            serialPortComboBox->currentData().toString());
        fw->open();
    }
    catch (std::runtime_error e) {
        QMessageBox::critical(this, "Runtime error", e.what());
    }
}

void FilterWheelWidget::disconnectDevice()
{
    try {
        fw->close();
        filterComboBox->clear();
        motionEnabled = false;
    }
    catch (std::runtime_error e) {
        QMessageBox::critical(this, "Runtime error", e.what());
    }
}

void FilterWheelWidget::refreshValues()
{
    try{
        filterLabel->setText(
            QString("Filter: %1").arg(fw->getPosition()));
    }
    catch (std::runtime_error e) {
        logger->error(e.what());
    }
}
