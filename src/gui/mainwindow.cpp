#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QCloseEvent>
#include <QToolBar>
#include <QLabel>
#include <QStatusBar>
#include <QSettings>
#include <QSerialPortInfo>

#include "core/spim.h"
#include "core/statemachine.h"
#include "core/logmanager.h"
#include "core/serialport.h"
#include "core/galvoramp.h"
#include "core/cameratrigger.h"

#include "mainwindow.h"
#include "centralwidget.h"
#include "settings.h"
#include "version.h"


static Logger *logger = getLogger("MainWindow");

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    loadSettings();

    setupUi();

    updateTimer.setInterval(500);
    connect(&updateTimer, &QTimer::timeout, this, &MainWindow::updatePIValues);

    QThread *thread = new QThread();
    spim().moveToThread(thread);
    thread->start();

    QMetaObject::connectSlotsByName(this);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    connect(&spim(), &SPIM::error, this, [ = ](QString s) {
        QMessageBox::critical(nullptr, "Error", s);
    });
    QAction *quitAction = new QAction(this);
    quitAction->setText("&Quit");
    quitAction->setObjectName("quitAction");
    quitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);

    QAction *aboutAction = new QAction(this);
    aboutAction->setText("&About...");
    aboutAction->setObjectName("aboutAction");
    aboutAction->setShortcut(Qt::Key_F1);

    QMenuBar *menuBar = new QMenuBar();
    setMenuBar(menuBar);

    QMenu *fileMenu = menuBar->addMenu("&File");
    fileMenu->addAction(quitAction);

    QMenu *helpMenu = menuBar->addMenu("?");
    helpMenu->addAction(aboutAction);

    CentralWidget *centralWidget = new CentralWidget(this);
    setCentralWidget(centralWidget);

    QLabel *statusLabel = new QLabel();
    statusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusBar()->addWidget(statusLabel);

    QState *s;

    s = stateMachine().getState(STATE_UNINITIALIZED);
    s->assignProperty(statusLabel, "text", "Uninitialized");

    s = stateMachine().getState(STATE_READY);
    s->assignProperty(statusLabel, "text", "Ready");
    void (QTimer::* mySlot)() = &QTimer::start;
    connect(s, &QState::entered, &updateTimer, mySlot);

    s = stateMachine().getState(STATE_CAPTURING);
    s->assignProperty(statusLabel, "text", "Capturing");
}

void MainWindow::saveSettings() const
{
    Settings mySettings = settings();

    QSettings settings;

    settings.beginGroup("MainWindow");
    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());
    settings.endGroup();

    QString group;

    for (int i = 0; i < 5; ++i) {
        PIDevice *dev = spim().getPIDevice(i);
        group = SETTINGSGROUP_AXIS(i);
        mySettings.setValue(group, SETTING_BAUD, dev->getBaud());
        mySettings.setValue(
            group, SETTING_DEVICENUMBER, dev->getDeviceNumber());
        mySettings.setValue(group, SETTING_PORTNAME, dev->getPortName());
        if (!dev->getPortName().isEmpty()) {
            QSerialPortInfo info(dev->getPortName());
            mySettings.setValue(
                group, SETTING_SERIALNUMBER, info.serialNumber());
        }
    }

    GalvoRamp *gr = spim().getGalvoRamp();
    group = SETTINGSGROUP_GRAMP;
    mySettings.setValue(group, SETTING_PHYSCHANS, gr->getPhysicalChannels());
    QList<QVariant> waveformParams;
    for (QVariant variant : spim().getGalvoRamp()->getWaveformParams()) {
        waveformParams.append(variant.toDouble());
    }
    mySettings.setValue(group, SETTING_WFPARAMS, waveformParams);

    CameraTrigger *ct = spim().getCameraTrigger();
    group = SETTINGSGROUP_CAMTRIG;
    mySettings.setValue(group, SETTING_PHYSCHANS, ct->getPhysicalChannels());
    mySettings.setValue(group, SETTING_TERMS, ct->getTerms());

    group = SETTINGSGROUP_SPIM;
    mySettings.setValue(group, SETTING_EXPTIME, spim().getExposureTime());
}

void MainWindow::loadSettings()
{
    const Settings mySettings = settings();

    QString group;

    QSettings settings;

    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    restoreState(settings.value("mainWindowState").toByteArray());
    settings.endGroup();

    for (int i = 0; i < 5; ++i) {
        PIDevice *dev = spim().getPIDevice(i);
        group = SETTINGSGROUP_AXIS(i);
        dev->setBaud(mySettings.value(group, SETTING_BAUD).toInt());
        dev->setDeviceNumber(
            mySettings.value(group, SETTING_DEVICENUMBER).toInt());
        QString sn = mySettings.value(group, SETTING_SERIALNUMBER).toString();
        if (!sn.isEmpty()) {
            QSerialPortInfo info = SerialPort::findPortFromSerialNumber(sn);
            if (!info.portName().isEmpty()) {
                dev->setPortName(info.portName());
            }
        }
        else {
            dev->setPortName(
                mySettings.value(group, SETTING_PORTNAME).toString());
        }
    }

    GalvoRamp *gr = spim().getGalvoRamp();
    group = SETTINGSGROUP_GRAMP;
    gr->setPhysicalChannels(
        mySettings.value(group, SETTING_PHYSCHANS).toStringList());
    QVector<double> wp;
    const QList<QVariant> wafeformParams =
        mySettings.value(group, SETTING_WFPARAMS).toList();
    for (int i = 0; i < wafeformParams.count(); i++) {
        wp << wafeformParams.at(i).toDouble();
    }
    gr->setWaveformParams(wp);

    group = SETTINGSGROUP_CAMTRIG;
    CameraTrigger *ct = spim().getCameraTrigger();
    ct->setPhysicalChannels(
        mySettings.value(group, SETTING_PHYSCHANS).toStringList());
    ct->setTerms(mySettings.value(group, SETTING_TERMS).toStringList());

    group = SETTINGSGROUP_SPIM;
    spim().setExposureTime(mySettings.value(group, SETTING_EXPTIME).toDouble());
}

void MainWindow::on_aboutAction_triggered() const
{
    QMessageBox msgBox;
    QString text;
    QTextStream ts(&text);

    ts << QString("<b>%1</b> ").arg(PROGRAM_NAME);
    ts << "An interface for our new SPIM microscope.<br><br>";
    ts << "<i>Version</i>:&nbsp;" << getProgramVersionString() << "<br>";
    ts << "<i>Authors</i>:<br>";
    ts << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Giacomo Mazzamuto<br>";
    ts << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Ludovico Silvestri<br>";
    ts << "<i>Date</i>:&nbsp; November 2018<br>";
    ts << "Qt version: " << qVersion();

    msgBox.setText(text);
    msgBox.setWindowTitle(QString("About %1").arg(PROGRAM_NAME));
    msgBox.setIconPixmap(QPixmap(":/res/logo_lens.png"));
    msgBox.exec();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
#ifdef WITH_HARDWARE
    QMessageBox::StandardButton ret = QMessageBox::question(
        this, QString("Closing %1").arg(PROGRAM_NAME),
        QString("Are you sure you want to close %1?").arg(PROGRAM_NAME),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (ret != QMessageBox::Yes) {
        e->ignore();
        return;
    }
#endif

    saveSettings();
    spim().uninitialize();
    QMainWindow::closeEvent(e);
}

void MainWindow::updatePIValues()
{
    for (PIDevice * dev : spim().getPIDevices()) {
        if (!dev->isConnected()) {
            continue;
        }
        dev->getCurrentPosition();
    }
}
