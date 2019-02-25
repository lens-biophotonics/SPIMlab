#include <QGroupBox>
#include <QGridLayout>
#include <QPushButton>
#include <QSerialPortInfo>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QState>
#include <QMessageBox>

#include <QtDebug>

#include "picontrollersettingswidget.h"

enum REFERENCE_ACTION {
    REFACTION_DONT_REFERENCE = 0,
    REFACTION_POS_LIMITS = 1,
    REFACTION_NEG_LIMITS = 2,
    REFACTION_REF_SWITCH = 3,
};

PIControllerSettingsWidget::PIControllerSettingsWidget(
    const QString &title,
    PIDevice *device,
    QWidget *parent) : QWidget(parent), device(device), title(title)
{
    setupUI();
    refreshValues();
}

void PIControllerSettingsWidget::configureStages()
{
    QDialog *dialog = new QDialog();

    dialog->setWindowTitle("Configure stages");

    QString axes = device->getAxisIdentifiers();

    QStringList availStages = device->getAvailableStageTypes();
    QStringList stages = device->getStages("", true);

    QFormLayout *formLayout = new QFormLayout();

    QList<QComboBox *> cbList;
    QList<QButtonGroup *> bgList;

    for (int i = 0; i < axes.count(); ++i) {
        QComboBox *cb = new QComboBox();
        cb->addItems(availStages);
        cb->setCurrentText(stages.at(i));
        cbList.append(cb);

        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->addWidget(cb);

        QButtonGroup *bg = new QButtonGroup();
        bgList.append(bg);

        QRadioButton *rb;

        rb = new QRadioButton("Don't ref.");
        rb->setChecked(true);
        bg->addButton(rb, REFACTION_DONT_REFERENCE);
        hLayout->addWidget(rb);

        rb = new QRadioButton("Pos. limits");
        bg->addButton(rb, REFACTION_POS_LIMITS);
        hLayout->addWidget(rb);

        rb = new QRadioButton("Neg. limits");
        bg->addButton(rb, REFACTION_NEG_LIMITS);
        hLayout->addWidget(rb);

        rb = new QRadioButton("Ref. switch");
        bg->addButton(rb, REFACTION_REF_SWITCH);
        hLayout->addWidget(rb);

        formLayout->addRow(new QLabel(QString("Axis %1").arg(axes.at(i))),
                           hLayout);
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    formLayout->addRow(buttonBox);

    dialog->setLayout(formLayout);
    if (dialog->exec() != QDialog::Accepted)
        return;

    QString newAxes;
    QStringList newStages;

    QString posAxes;
    QString negAxes;
    QString switchAxes;

    for (int i = 0; i < axes.count(); ++i) {
        const QString tmpStage = cbList.at(i)->currentText();
        const QChar c = axes.at(i);
        if (tmpStage != stages.at(i)) {
            newAxes.append(c);
            newStages.append(tmpStage);
        }

        switch (bgList.at(i)->checkedId()) {
        case REFACTION_POS_LIMITS:
            posAxes.append(c);
            break;

        case REFACTION_NEG_LIMITS:
            negAxes.append(c);
            break;

        case REFACTION_REF_SWITCH:
            switchAxes.append(c);
            break;

        case REFACTION_DONT_REFERENCE:
        default:
            break;
        }
    }

    device->loadStages(newAxes, newStages);

    QString svoAxes = posAxes + negAxes + switchAxes;

    device->setServoEnabled(svoAxes, QVector<int>(svoAxes.count(), 1));
    device->fastMoveToPositiveLimit(posAxes);
    device->fastMoveToNegativeLimit(negAxes);
    device->fastMoveToReferenceSwitch(switchAxes);
    refreshValues();
}

void PIControllerSettingsWidget::setupUI()
{
    QGridLayout *grid = new QGridLayout();

    serialPortComboBox = new QComboBox();
    QSize size = serialPortComboBox->size();
    size.setWidth(150);
    serialPortComboBox->setMaximumSize(size);
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        if (info.manufacturer() != "PI") {
            continue;
        }
        QString descr = QString("%1 (%2, %3)")
                        .arg(info.description())
                        .arg(info.portName())
                        .arg(info.serialNumber());
        serialPortComboBox->addItem(descr, info.portName());
    }

    int row = 0;
    grid->addWidget(new QLabel("Serial port"), row, 0, 1, 1);
    grid->addWidget(serialPortComboBox, row++, 1, 1, 1);

    baudComboBox = new QComboBox();
    baudComboBox->setEnabled(false);
    baudComboBox->addItem("Don't set (not primary DC)", -1);
    baudComboBox->addItem("9600", 9600);
    baudComboBox->addItem("19200", 19200);
    baudComboBox->addItem("38400", 38400);
    baudComboBox->addItem("115200", 115200);
    grid->addWidget(new QLabel("Baud rate"), row, 0, 1, 1);
    grid->addWidget(baudComboBox, row++, 1, 1, 1);

    deviceNumberSpinBox = new QSpinBox();
    deviceNumberSpinBox->setMinimum(1);
    deviceNumberSpinBox->setMaximum(17);
    deviceNumberSpinBox->setValue(device->getDeviceNumber());
    grid->addWidget(new QLabel("Device number"), row, 0, 1, 1);
    grid->addWidget(deviceNumberSpinBox, row++, 1, 1, 1);

    QPushButton *connectPushButton = new QPushButton("Connect");
    QPushButton *disconnectPushButton = new QPushButton("Disconnect");
    grid->addWidget(connectPushButton, row, 0, 1, 1);
    grid->addWidget(disconnectPushButton, row++, 1, 1, 1);

    QFrame *line;
    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    grid->addWidget(line, row++, 0, 1, 2);

    QFont monospaced("Monospace");
    monospaced.setStyleHint(QFont::TypeWriter);

    axisIdentifiersLabel = new QLabel();
    axisIdentifiersLabel->setFont(monospaced);
    grid->addWidget(new QLabel("Axes"), row, 0, 1, 1);
    grid->addWidget(axisIdentifiersLabel, row++, 1, 1, 1);

    referencedStateLabel = new QLabel();
    referencedStateLabel->setFont(monospaced);
    grid->addWidget(new QLabel("Referenced"), row, 0, 1, 1);
    grid->addWidget(referencedStateLabel, row++, 1, 1, 1);

    stagesLabel = new QLabel();
    grid->addWidget(new QLabel("Stages"), row, 0, 1, 1);
    grid->addWidget(stagesLabel, row++, 1, 1, 1);

    QPushButton *configureStagesPushButton =
        new QPushButton("Configure stages...");
    grid->addWidget(configureStagesPushButton, row++, 1, 1, 1);

    QPushButton *refreshPushButton = new QPushButton("Refresh");
    grid->addWidget(refreshPushButton, row++, 0, 1, 2);

    QGroupBox *groubBox = new QGroupBox(title);
    groubBox->setLayout(grid);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addWidget(groubBox);
    vlayout->setMargin(0);

    setLayout(vlayout);

    connect(configureStagesPushButton, &QPushButton::clicked,
            this, &PIControllerSettingsWidget::configureStages);
    connect(refreshPushButton, &QPushButton::clicked,
            this, &PIControllerSettingsWidget::refreshValues);

    connect(connectPushButton, &QPushButton::clicked,
            this, &PIControllerSettingsWidget::connectDevice);
    connect(disconnectPushButton, &QPushButton::clicked,
            device, &PIDevice::close);

    connect(device, &PIDevice::connected, this,
            &PIControllerSettingsWidget::refreshValues);

    QState *cs = device->connectedState();
    QState *ds = device->disconnectedState();

    QList<QWidget *> wList;

    // enabled when connected, disabled when disconnected
    wList = {
        refreshPushButton,
        configureStagesPushButton,
        disconnectPushButton,
    };

    foreach (QWidget * w, wList) {
        cs->assignProperty(w, "enabled", true);
        ds->assignProperty(w, "enabled", false);
    }

    // enabled when disconnected, disabled when connected
    wList = {
        connectPushButton,
        serialPortComboBox,
        deviceNumberSpinBox,
        baudComboBox,
    };

    foreach (QWidget * w, wList) {
        cs->assignProperty(w, "enabled", false);
        ds->assignProperty(w, "enabled", true);
    }

    refreshValues();
}

void PIControllerSettingsWidget::refreshValues()
{
    const QString portName = device->getPortName();
    int idx;
    idx = serialPortComboBox->findData(device->getPortName());
    serialPortComboBox->setCurrentIndex(idx);

    idx = baudComboBox->findData(device->getBaud());
    baudComboBox->setCurrentIndex(idx);

    deviceNumberSpinBox->setValue(device->getDeviceNumber());

    if (!device->isConnected()) {
        return;
    }

    const QString axes = device->getAxisIdentifiers();
    const QVector<int> rs = device->getReferencedState();
    const QString stages = device->getStages().join("\n");

    axisIdentifiersLabel->setText(axes);

    QString rsTxt;
    foreach(const int state, rs) {
        rsTxt += state ? "Y" : "N";
    }
    referencedStateLabel->setText(rsTxt);
    stagesLabel->setText(stages);
}

void PIControllerSettingsWidget::connectDevice()
{
    int baud = baudComboBox->currentData().toInt();

    try {
        device->close();
        device->connectDaisyChainSerial(
            serialPortComboBox->currentData().toString(),
            deviceNumberSpinBox->value(),
            baud);
    }
    catch (std::runtime_error e) {
        QMessageBox::critical(this, "Runtime error", e.what());
    }
}
