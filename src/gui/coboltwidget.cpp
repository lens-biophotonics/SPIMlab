#include <QGroupBox>
#include <QGridLayout>
#include <QListView>
#include <QSerialPortInfo>
#include <QPushButton>
#include <QState>
#include <QMessageBox>
#include <QTimer>

#include <qtlab/hw/serial/cobolt.h>
#include <qtlab/hw/serial/serialport.h>
#include <qtlab/core/logger.h>

#include <qtlab/widgets/customspinbox.h>

#include "coboltwidget.h"
#include "utils.h"

static Logger *logger = getLogger("SerialPort");

CoboltWidget::CoboltWidget(Cobolt *cobolt, QWidget *parent) :
    QWidget(parent), cobolt(cobolt)
{
    setupUI();

    QTimer *refreshTimer = new QTimer();
    refreshTimer->setInterval(2000);
    connect(refreshTimer, &QTimer::timeout, this, &CoboltWidget::refreshValues);

    connect(cobolt, &Cobolt::connected, [ = ](){
        refreshTimer->start();
    });
    connect(cobolt, &Cobolt::disconnected, [ = ](){
        refreshTimer->stop();
    });
}

void CoboltWidget::setupUI()
{
    QGridLayout *grid = new QGridLayout();

    serialPortComboBox = new QComboBox();
    QListView *view = new QListView();
    view->setFixedWidth(350);
    serialPortComboBox->setView(view);
    serialPortComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    serialPortComboBox->setMinimumContentsLength(15);

    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        if (!info.manufacturer().startsWith("Cobolt")) {
            continue;
        }
        QString descr = QString("%1 (%2, %3)")
                        .arg(info.portName())
                        .arg(info.description())
                        .arg(info.serialNumber());
        serialPortComboBox->addItem(descr, info.portName());
    }

    serialPortComboBox->setCurrentIndex(
        serialPortComboBox->findData(cobolt->getSerialPort()->portName()));

    serialPortComboBox->model()->sort(0);

    int row = 0;
    grid->addWidget(new QLabel("Serial port"), row, 0, 1, 1);
    grid->addWidget(serialPortComboBox, row++, 1, 1, 1);

    QPushButton *connectPushButton = new QPushButton("Connect");
    QPushButton *disconnectPushButton = new QPushButton("Disconnect");
    grid->addWidget(connectPushButton, row, 0, 1, 1);
    grid->addWidget(disconnectPushButton, row++, 1, 1, 1);

    QFrame *line;
    line = new QFrame();
    line->setFrameShape(QFrame::StyledPanel);
    line->setFrameShadow(QFrame::Sunken);
    line->setMinimumHeight(7);
    grid->addWidget(line, row++, 0, 1, 2);

    QPushButton *onPushButton = new QPushButton("On");
    QPushButton *offPushButton = new QPushButton("Off");

    grid->addWidget(onPushButton, row, 0, 1, 1);
    grid->addWidget(offPushButton, row++, 1, 1, 1);

    powerLabel = new QLabel("Power:");
    DoubleSpinBox *powerDoubleSpinBox = new DoubleSpinBox();
    powerDoubleSpinBox->setMaximum(250);
    powerDoubleSpinBox->setSuffix(" mW");
    powerDoubleSpinBox->setDecimals(2);

    grid->addWidget(powerLabel, row, 0, 1, 1);
    grid->addWidget(powerDoubleSpinBox, row++, 1, 1, 1);

    QGroupBox *gb = new QGroupBox("Laser");
    gb->setLayout(grid);
    QBoxLayout *bl = new QHBoxLayout();
    bl->addWidget(gb);
    setLayout(bl);

    connect(connectPushButton, &QPushButton::clicked,
            this, &CoboltWidget::connectDevice);
    connect(disconnectPushButton, &QPushButton::clicked,
            cobolt, &Cobolt::close);

    connect(onPushButton, &QPushButton::clicked, this, [ = ](){
        try {
            cobolt->setLaserOn();
        }
        catch (std::runtime_error e) {
            QMessageBox::critical(this, "Runtime error", e.what());
        }
        onPushButton->setEnabled(false);
        offPushButton->setEnabled(true);
    });
    connect(offPushButton, &QPushButton::clicked, this, [ = ](){
        try {
            cobolt->setLaserOff();
        }
        catch (std::runtime_error e) {
            QMessageBox::critical(this, "Runtime error", e.what());
        }
        onPushButton->setEnabled(true);
        offPushButton->setEnabled(false);
    });

    connect(cobolt, &Cobolt::connected, this, [ = ](){
        bool on = false;
        try {
            on = cobolt->getOnOffState();
            powerDoubleSpinBox->setValue(
                cobolt->getOutputPowerSetPoint() * 1000);
            int wl = cobolt->getWavelength();
            gb->setTitle(QString("%1 nm").arg(wl));
            line->setStyleSheet(
                "background-color: " + wavelengthToColor(wl).name());
        }
        catch (std::runtime_error) {
        }
        onPushButton->setEnabled(!on);
        offPushButton->setEnabled(on);
        refreshValues();
    });

    connect(powerDoubleSpinBox, &DoubleSpinBox::returnPressed, this, [ = ](){
        try {
            cobolt->setOutputPower(powerDoubleSpinBox->value() / 1000.);
        }
        catch (std::runtime_error e) {
            QMessageBox::critical(this, "Runtime error", e.what());
        }
    });

    QState *cs = cobolt->getSerialPort()->getConnectedState();
    QState *ds = cobolt->getSerialPort()->getDisconnectedState();

    ds->assignProperty(line, "styleSheet", "background-color: gray");

    QList<QWidget *> wList;

    // enabled when connected, disabled when disconnected
    wList = {
        disconnectPushButton,
        onPushButton,
        offPushButton,
        powerDoubleSpinBox,
        powerLabel,
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

void CoboltWidget::connectDevice()
{
    try {
        cobolt->getSerialPort()->setPortName(
            serialPortComboBox->currentData().toString());
        cobolt->open();
    }
    catch (std::runtime_error e) {
        QMessageBox::critical(this, "Runtime error", e.what());
    }
}

void CoboltWidget::refreshValues()
{
    try{
        powerLabel->setText(
            QString("Power: %1 mW").arg(cobolt->getOutputPower() * 1000));
    }
    catch (std::runtime_error e) {
        logger->error(e.what());
    }
}
