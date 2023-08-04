#include "thorlabsmcwidget.h"

#include <qtlab/core/logger.h>
#include <qtlab/widgets/customspinbox.h>

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListView>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSerialPortInfo>
#include <QState>

static Logger *logger = getLogger("SerialPort");

#define STEPS_PER_DEG 1919.6418578623391

ThorlabsMCWidget::ThorlabsMCWidget(MotorController *mc, QWidget *parent)
    : QWidget(parent)
    , mc(mc)
{
    setupUI();

    connect(mc, &MotorController::error, this, [=](QString e) {
        QMessageBox::critical(this, "Runtime error", e);
    });
}

void ThorlabsMCWidget::setupUI()
{
    QGridLayout *grid = new QGridLayout();

    QComboBox *serialPortComboBox = new QComboBox();
    QListView *view = new QListView();
    view->setFixedWidth(350);
    serialPortComboBox->setView(view);
    serialPortComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    serialPortComboBox->setMinimumContentsLength(15);

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

    serialPortComboBox->model()->sort(0);
    if (serialPortComboBox->count() == 1) {
        serialPortComboBox->setCurrentIndex(0);
    } else {
        serialPortComboBox->setCurrentIndex(
            serialPortComboBox->findData(mc->serialPort()->portName()));
    }

    int row = 0;
    grid->addWidget(new QLabel("Serial port"), row, 0, 1, 1);
    grid->addWidget(serialPortComboBox, row++, 1, 1, 1);

    QPushButton *connectPushButton = new QPushButton("Connect");
    QPushButton *disconnectPushButton = new QPushButton("Disconnect");
    grid->addWidget(connectPushButton, row, 0, 1, 1);
    grid->addWidget(disconnectPushButton, row++, 1, 1, 1);

    QLabel *angleLabel = new QLabel("Angle:");
    DoubleSpinBox *angleDoubleSpinBox = new DoubleSpinBox();
    angleDoubleSpinBox->setMaximum(250);
    angleDoubleSpinBox->setSuffix(" °");
    angleDoubleSpinBox->setDecimals(2);

    QPushButton *homePushButton = new QPushButton("Home");
    QLabel *modelLabel = new QLabel("Model:");

    grid->addWidget(modelLabel, row, 0, 1, 1);
    grid->addWidget(homePushButton, row++, 1, 1, 1);
    grid->addWidget(angleLabel, row, 0, 1, 1);
    grid->addWidget(angleDoubleSpinBox, row++, 1, 1, 1);

    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);

    QProgressBar *pb = new QProgressBar();

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(grid);
    vLayout->addWidget(pb);

    setLayout(vLayout);

    QList<QWidget *> wList;

    wList = {
        modelLabel,
        homePushButton,
        angleLabel,
        angleDoubleSpinBox,
        disconnectPushButton,
    };

    auto updateAngle = [=]() {
        angleLabel->setText(
            QString("Angle: %1°").arg(mc->getPositionCounter() / STEPS_PER_DEG, 0, 'f', 2));
    };

    auto enableControls = [=]() {
        for (QWidget *w : wList) {
            w->setEnabled(true);
        }
        pb->setRange(0, 100);
        updateAngle();
    };

    auto disableControls = [=]() {
        for (QWidget *w : wList) {
            w->setEnabled(false);
        }
        pb->setRange(0, 0);
        angleLabel->setText("Angle:");
    };

    connect(mc, &MotorController::movedHome, mc, enableControls);
    connect(mc, &MotorController::moveCompleted, mc, enableControls);
    connect(mc, &MotorController::moveStopped, mc, enableControls);

    connect(mc, &SerialDevice::connected, mc, [=]() {
        modelLabel->setText(QString("Model: ") + mc->getHwInfo()->ModelNumber());
        updateAngle();
    });

    connect(
        connectPushButton,
        &QPushButton::clicked,
        this,
        [=]() {
            mc->serialPort()->setPortName(serialPortComboBox->currentData().toString());
            connectPushButton->setEnabled(false);
        },
        Qt::DirectConnection);
    connect(connectPushButton, &QPushButton::clicked, mc, &MotorController::connect);

    connect(disconnectPushButton, &QPushButton::clicked, [=]() {
        disconnectPushButton->setEnabled(false);
    });
    connect(disconnectPushButton, &QPushButton::clicked, mc, &MotorController::disconnect);

    connect(angleDoubleSpinBox, &DoubleSpinBox::returnPressed, mc, [=]() {
        mc->startAbsoluteMove(STEPS_PER_DEG * angleDoubleSpinBox->value());
    });
    connect(angleDoubleSpinBox, &DoubleSpinBox::returnPressed, disableControls);

    connect(homePushButton, &QPushButton::clicked, disableControls);
    connect(homePushButton, &QPushButton::clicked, mc, [=]() { mc->moveToHome(); });

    QState *cs = mc->serialPort()->getConnectedState();
    QState *ds = mc->serialPort()->getDisconnectedState();

    // enabled when connected, disabled when disconnected
    wList << pb;

    for (QWidget *w : wList) {
        cs->assignProperty(w, "enabled", true);
        ds->assignProperty(w, "enabled", false);
    }

    // enabled when disconnected, disabled when connected
    wList = {
        connectPushButton,
        serialPortComboBox,
    };

    for (QWidget *w : wList) {
        cs->assignProperty(w, "enabled", false);
        ds->assignProperty(w, "enabled", true);
    };
}

void ThorlabsMCWidget::refreshValues()
{
    try {
    } catch (std::runtime_error e) {
        logger->critical(e.what());
    }
}
