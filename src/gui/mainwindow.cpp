#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QCloseEvent>
#include <QToolBar>
#include <QSettings>
#include <QSerialPortInfo>

#include <qtlab/core/logmanager.h>
#include <qtlab/hw/serial/serialport.h>
#include <qtlab/hw/serial/cobolt.h>
#include <qtlab/hw/serial/filterwheel.h>
#include <qtlab/hw/serial/AA_MPDSnCxx.h>

#include "spim.h"
#include "galvoramp.h"
#include "cameratrigger.h"

#include "mainwindow.h"
#include "centralwidget.h"
#include "settings.h"
#include "version.h"


static Logger *logger = getLogger("MainWindow");

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    loadSettings();

    setupUi();

    QThread *thread = new QThread();
    thread->setObjectName("SPIM_thread");
    spim().moveToThread(thread);
    thread->start();

    QMetaObject::connectSlotsByName(this);
    logManager().flushMessages();
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

    QAction *saveSettingsAction = new QAction(this);
    saveSettingsAction->setText("Save settings");
    saveSettingsAction->setShortcut(QKeySequence("Ctrl+S"));
    connect(saveSettingsAction, &QAction::triggered,
            this, &MainWindow::saveSettings);

    QAction *aboutAction = new QAction(this);
    aboutAction->setText("&About...");
    aboutAction->setObjectName("aboutAction");
    aboutAction->setShortcut(Qt::Key_F1);

    QMenuBar *menuBar = new QMenuBar();
    setMenuBar(menuBar);

    QMenu *fileMenu = menuBar->addMenu("&File");
    fileMenu->addAction(saveSettingsAction);
    fileMenu->addAction(quitAction);

    QMenu *helpMenu = menuBar->addMenu("?");
    helpMenu->addAction(aboutAction);

    CentralWidget *centralWidget = new CentralWidget(this);
    setCentralWidget(centralWidget);
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

    for (int i = 0; i < SPIM_NPIDEVICES; ++i) {
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

        QList<double> *scanRange =
            spim().getScanRange(static_cast<SPIM_PI_DEVICES>(i));
        mySettings.setValue(group, SETTING_FROM, scanRange->at(0));
        mySettings.setValue(group, SETTING_TO, scanRange->at(1));
        mySettings.setValue(group, SETTING_STEP, scanRange->at(2));
    }

    for (int i = 0; i < SPIM_NCOBOLT; ++i) {
        SerialPort *sp = spim().getLaser(i)->getSerialPort();
        group = SETTINGSGROUP_COBOLT(i);
        mySettings.setValue(group, SETTING_PORTNAME, sp->portName());
    }

    for (int i = 0; i < SPIM_NFILTERWHEEL; ++i) {
        SerialPort *sp = spim().getFilterWheel(i)->getSerialPort();
        group = SETTINGSGROUP_FILTERWHEEL(i);
        mySettings.setValue(group, SETTING_SERIALNUMBER, sp->portInfo().serialNumber());
    }

    GalvoRamp *gr = spim().getGalvoRamp();
    group = SETTINGSGROUP_GRAMP;
    mySettings.setValue(group, SETTING_PHYSCHANS, gr->getPhysicalChannels());
    mySettings.setValue(group, SETTING_TRIGGER_TERM, gr->getTriggerTerm());
    QList<QVariant> waveformParams;
    for (QVariant variant : spim().getGalvoRamp()->getWaveformParams()) {
        waveformParams.append(variant.toDouble());
    }
    mySettings.setValue(group, SETTING_WFPARAMS, waveformParams);

    CameraTrigger *ct = spim().getCameraTrigger();
    group = SETTINGSGROUP_CAMTRIG;
    mySettings.setValue(group, SETTING_PHYSCHANS, ct->getPhysicalChannels());
    mySettings.setValue(group, SETTING_TRIGGER_TERM, ct->getTriggerTerm());

    group = SETTINGSGROUP_ACQUISITION;
    mySettings.setValue(group, SETTING_EXPTIME, spim().getExposureTime());
    mySettings.setValue(group, SETTING_OUTPUTPATH, spim().getOutputPath());

    group = SETTINGSGROUP_OTHERSETTINGS;
    mySettings.setValue(group, SETTING_SCANVELOCITY, spim().getScanVelocity());

    mySettings.saveSettings();
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

    for (int i = 0; i < SPIM_NPIDEVICES; ++i) {
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

        QList<double> *scanRange =
            spim().getScanRange(static_cast<SPIM_PI_DEVICES>(i));
        scanRange->replace(0, mySettings.value(group, SETTING_FROM).toDouble());
        scanRange->replace(1, mySettings.value(group, SETTING_TO).toDouble());
        scanRange->replace(2, mySettings.value(group, SETTING_STEP).toDouble());
    }

    for (int i = 0; i < SPIM_NCOBOLT; ++i) {
        Cobolt *dev = spim().getLaser(i);
        group = SETTINGSGROUP_COBOLT(i);
        dev->getSerialPort()->setPortName(
            mySettings.value(group, SETTING_PORTNAME).toString());
    }

    for (int i = 0; i < SPIM_NFILTERWHEEL; ++i) {
        FilterWheel *dev = spim().getFilterWheel(i);
        group = SETTINGSGROUP_FILTERWHEEL(i);
        dev->getSerialPort()->setPortBySerialNumber(
            mySettings.value(group, SETTING_SERIALNUMBER).toString());
    }

    GalvoRamp *gr = spim().getGalvoRamp();
    group = SETTINGSGROUP_GRAMP;
    gr->setPhysicalChannels(
        mySettings.value(group, SETTING_PHYSCHANS).toStringList());
    gr->setTriggerTerm(
        mySettings.value(group, SETTING_TRIGGER_TERM).toString());
    QVector<double> wp;
    const QList<QVariant> wafeformParams =
        mySettings.value(group, SETTING_WFPARAMS).toList();
    gr->resetWaveFormParams(SPIM_NCAMS);
    for (int i = 0; i < wafeformParams.count(); i++) {
        wp << wafeformParams.at(i).toDouble();
    }
    gr->setWaveformParams(wp);

    group = SETTINGSGROUP_CAMTRIG;
    CameraTrigger *ct = spim().getCameraTrigger();
    ct->setPhysicalChannels(
        mySettings.value(group, SETTING_PHYSCHANS).toStringList());
    ct->setTriggerTerm(mySettings.value(group, SETTING_TRIGGER_TERM).toString());

    group = SETTINGSGROUP_ACQUISITION;
    spim().setExposureTime(mySettings.value(group, SETTING_EXPTIME).toDouble());
    spim().setOutputPath(mySettings.value(group, SETTING_OUTPUTPATH).toString());

    group = SETTINGSGROUP_OTHERSETTINGS;
    spim().setScanVelocity(
        mySettings.value(group, SETTING_SCANVELOCITY).toDouble());
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
    ts << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Vladislav Gavryusev<br>";
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

    /* SPIM::uninitialize() is called in the receiver's thread (i.e. spim's).
     * This ensures that QTimers can be correctly stopped (they can't be
     * stopped from a different thread), etc.
     * The invokation is blocking, so that there is no risk that spim is
     * destroyed before uninitialize() is called.
     */

    QMetaObject::invokeMethod(&spim(), "uninitialize",
                              Qt::BlockingQueuedConnection);
    QMainWindow::closeEvent(e);
}
