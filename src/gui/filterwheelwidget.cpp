#include <QGroupBox>
#include <QGridLayout>
#include <QListView>
#include <QSerialPortInfo>
#include <QPushButton>
#include <QState>
#include <QMessageBox>
#include <QTimer>

#include "core/filterwheel.h"
#include "core/serialport.h"
#include "core/logger.h"

#include "filterwheelwidget.h"
#include "customspinbox.h"
#include "utils.h"

static Logger *logger = getLogger("SerialPort");

FilterWheelWidget::FilterWheelWidget(FilterWheel *fw, QWidget *parent) :
    QWidget(parent), fw(fw)
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

    QPushButton *connectPushButton = new QPushButton("Connect");
    QPushButton *disconnectPushButton = new QPushButton("Disconnect");
    grid->addWidget(connectPushButton, row, 0, 1, 1);
    grid->addWidget(disconnectPushButton, row++, 1, 1, 1);

    filterLabel = new QLabel("Filter:");
    filterComboBox = new QComboBox();
    QListView *viewFilter = new QListView();
    viewFilter->setFixedWidth(350);
    filterComboBox->setView(viewFilter);
    filterComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    filterComboBox->setMinimumContentsLength(15);

    grid->addWidget(filterLabel, row, 0, 1, 1);
    grid->addWidget(filterComboBox, row++, 1, 1, 1);

    QGroupBox *gb = new QGroupBox("Filter wheel");
    //QGroupBox *gb = new QGroupBox(device->getVerboseName()); // to have assigned device name
    //gb->setStyleSheet("QGroupBox{padding-top:1em; margin-top:-1em}"); // to make title disappear
    gb->setLayout(grid);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addWidget(gb);
    //vlayout->setMargin(0);

    setLayout(vlayout);

    connect(connectPushButton, &QPushButton::clicked,
            this, &FilterWheelWidget::connectDevice);
    connect(disconnectPushButton, &QPushButton::clicked,
            this, &FilterWheelWidget::disconnectDevice);

    connect(fw, &FilterWheel::connected, this, [ = ](){
        try {
            QString currfilter = fw->getFilterName(fw->getPosition());
            logger->info(QString("Current filter: %1").arg(currfilter));
            filterComboBox->addItems(fw->getFilterListName());
            filterComboBox->setCurrentIndex(filterComboBox->findText(currfilter));
            motionEnabled = true;
        }
        catch (std::runtime_error) {
        }
        refreshValues();
    });

    connect(filterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [ = ](){
        try {
            //logger->info(QString("current index %1").arg(filterComboBox->currentIndex()));
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
