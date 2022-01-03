#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QCloseEvent>
#include <QToolBar>
#include <QSettings>
#include <QSerialPortInfo>

#include <qtlab/core/logmanager.h>
#include <qtlab/hw/ni/natinst.h>
#include <qtlab/hw/serial/serialport.h>
#include <qtlab/hw/serial/cobolt.h>
#include <qtlab/hw/serial/filterwheel.h>
#include <qtlab/hw/serial/AA_MPDSnCxx.h>

#include <qwt_global.h>

#include "spim.h"
#include "cameratrigger.h"
#include "galvoramp.h"
#include "tasks.h"

#include "mainwindow.h"
#include "centralwidget.h"
#include "settings.h"
#include "version.h"

static Logger *logger = getLogger("MainWindow");

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    loadSettings();

    setupUi();
    setWindowIcon(QIcon(":/res/spim-icon.svg"));

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
    Settings s = settings();

    QSettings settings;

    settings.beginGroup("MainWindow");
    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());
    settings.endGroup();

    QString group;

    for (int i = 0; i < SPIM_NPIDEVICES; ++i) {
        PIDevice *dev = spim().getPIDevice(i);
        group = SETTINGSGROUP_AXIS(i);
        s.setValue(group, SETTING_BAUD, dev->getBaud());
        s.setValue(group, SETTING_DEVICENUMBER, dev->getDeviceNumber());
        s.setValue(group, SETTING_PORTNAME, dev->getPortName());
        if (!dev->getPortName().isEmpty()) {
            QSerialPortInfo info(dev->getPortName());
            s.setValue(group, SETTING_SERIALNUMBER, info.serialNumber());
        }

        SPIM_PI_DEVICES d_enum = static_cast<SPIM_PI_DEVICES>(i);
        QList<double> *scanRange = spim().getScanRange(d_enum);
        s.setValue(group, SETTING_FROM, scanRange->at(0));
        s.setValue(group, SETTING_TO, scanRange->at(1));
        s.setValue(group, SETTING_STEP, scanRange->at(2));

        s.setValue(group, SETTING_MOSAIC_ENABLED, spim().isMosaicStageEnabled(d_enum));
    }

    for (int i = 0; i < SPIM_NCOBOLT; ++i) {
        SerialPort *sp = spim().getLaser(i)->serialPort();
        group = SETTINGSGROUP_COBOLT(i);
        s.setValue(group, SETTING_PORTNAME, sp->portName());
    }

    for (int i = 0; i < SPIM_NCAMS; ++i) {
        SerialPort *sp = spim().getFilterWheel(i)->serialPort();
        group = SETTINGSGROUP_FILTERWHEEL(i);
        s.setValue(group, SETTING_SERIALNUMBER, sp->portInfo().serialNumber());
    }

    for (int i = 0; i < SPIM_NAOTF; ++i) {
        SerialPort *sp = spim().getAOTF(i)->serialPort();
        group = SETTINGSGROUP_AOTF(i);
        QString sn = sp->portInfo().serialNumber();
        if (!sn.isEmpty()) {
            s.setValue(group, SETTING_SERIALNUMBER, sn);
        }
    }

    GalvoRamp *gr = spim().getTasks()->getGalvoRamp();
    group = SETTINGSGROUP_GRAMP;
    s.setValue(group, SETTING_PHYSCHANS, gr->getPhysicalChannels());
    QList<QVariant> waveformParams;
    for (QVariant variant : gr->getWaveformParams()) {
        waveformParams.append(variant.toDouble());
    }
    s.setValue(group, SETTING_WFPARAMS, waveformParams);
    CameraTrigger *ct = spim().getTasks()->getCameraTrigger();
    group = SETTINGSGROUP_CAMTRIG;
    s.setValue(group, SETTING_PULSE_TERMS, ct->getPulseTerms());
    s.setValue(group, SETTING_BLANKING_TERMS, ct->getBlankingPulseTerms());
    s.setValue(group, SETTING_TRIGGER_TERM, ct->getStartTriggerTerm());

    group = SETTINGSGROUP_ACQUISITION;
    s.setValue(group, SETTING_EXPTIME, spim().getExposureTime());
    s.setValue(group, SETTING_RUN_NAME, spim().getRunName());
    s.setValue(group, SETTING_BINNING, spim().getBinning());

    group = SETTINGSGROUP_OTHERSETTINGS;
    s.setValue(group, SETTING_SCANVELOCITY, spim().getScanVelocity());
    s.setValue(group, SETTING_CAM_OUTPUT_PATH_LIST, spim().getOutputPathList());

    s.saveSettings();
}

void MainWindow::loadSettings()
{
    const Settings s = settings();

    QString group;

    QSettings settings;

    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    restoreState(settings.value("mainWindowState").toByteArray());
    settings.endGroup();

    for (int i = 0; i < SPIM_NPIDEVICES; ++i) {
        PIDevice *dev = spim().getPIDevice(i);
        group = SETTINGSGROUP_AXIS(i);
        dev->setBaud(s.value(group, SETTING_BAUD).toInt());
        dev->setDeviceNumber(s.value(group, SETTING_DEVICENUMBER).toInt());
        QString sn = s.value(group, SETTING_SERIALNUMBER).toString();
        if (!sn.isEmpty()) {
            QSerialPortInfo info = SerialPort::findPortFromSerialNumber(sn);
            if (!info.portName().isEmpty()) {
                dev->setPortName(info.portName());
            }
        }
        else {
            dev->setPortName(s.value(group, SETTING_PORTNAME).toString());
        }

        SPIM_PI_DEVICES d_enum = static_cast<SPIM_PI_DEVICES>(i);

        QList<double> *scanRange = spim().getScanRange(d_enum);
        scanRange->replace(0, s.value(group, SETTING_FROM).toDouble());
        scanRange->replace(1, s.value(group, SETTING_TO).toDouble());
        scanRange->replace(2, s.value(group, SETTING_STEP).toDouble());

        spim().setMosaicStageEnabled(d_enum, s.value(group, SETTING_MOSAIC_ENABLED).toBool());
    }

    for (int i = 0; i < SPIM_NCOBOLT; ++i) {
        Cobolt *dev = spim().getLaser(i);
        group = SETTINGSGROUP_COBOLT(i);
        dev->serialPort()->setPortName(s.value(group, SETTING_PORTNAME).toString());
    }

    for (int i = 0; i < SPIM_NCAMS; ++i) {
        FilterWheel *dev = spim().getFilterWheel(i);
        group = SETTINGSGROUP_FILTERWHEEL(i);
        dev->serialPort()->setPortBySerialNumber(s.value(group, SETTING_SERIALNUMBER).toString());
    }

    GalvoRamp *gr = spim().getTasks()->getGalvoRamp();
    group = SETTINGSGROUP_GRAMP;
    gr->setPhysicalChannels(s.value(group, SETTING_PHYSCHANS).toStringList());
    QVector<double> wp;
    const QList<QVariant> wafeformParams = s.value(group, SETTING_WFPARAMS).toList();
    gr->resetWaveFormParams(SPIM_NCAMS);
    for (int i = 0; i < wafeformParams.count(); i++) {
        wp << wafeformParams.at(i).toDouble();
    }
    gr->setWaveformParams(wp);

    for (int i = 0; i < SPIM_NAOTF; ++i) {
        AA_MPDSnCxx *dev = spim().getAOTF(i);
        group = SETTINGSGROUP_AOTF(i);
        dev->serialPort()->setPortBySerialNumber(s.value(group, SETTING_SERIALNUMBER).toString());
    }

    group = SETTINGSGROUP_CAMTRIG;
    CameraTrigger *ct = spim().getTasks()->getCameraTrigger();
    ct->setPulseTerms(s.value(group, SETTING_PULSE_TERMS).toStringList());
    ct->setBlankingPulseTerms(s.value(group, SETTING_BLANKING_TERMS).toStringList());
    ct->setStartTriggerTerm(s.value(group, SETTING_TRIGGER_TERM).toString());

    group = SETTINGSGROUP_ACQUISITION;
    spim().setExposureTime(s.value(group, SETTING_EXPTIME).toDouble());
    spim().setRunName(s.value(group, SETTING_RUN_NAME).toString());
    spim().setBinning(s.value(group, SETTING_BINNING).toUInt());

    group = SETTINGSGROUP_OTHERSETTINGS;
    spim().setScanVelocity(s.value(group, SETTING_SCANVELOCITY).toDouble());
    spim().setOutputPathList(s.value(group, SETTING_CAM_OUTPUT_PATH_LIST).toStringList());
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
    ts << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Vladislav Gavryusev<br>";
    ts << "<i>Date</i>:&nbsp; November 2018 &mdash; July 2021<br><br>";
    ts << "Qt version: " << qVersion() << "<br>";
    ts << "Qwt version: " << QWT_VERSION_STR << "<br>";
    ts << "NIDAQmx version: " << NI::getVersion() << "<br>";

    msgBox.setText(text);
    msgBox.setWindowTitle(QString("About %1").arg(PROGRAM_NAME));
    msgBox.setIconPixmap(QPixmap(":/res/logos.png"));
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

    /* This will trigger saveSettings() in the relevant widgets (e.g. CameraPage). This must happen
     * before MainWindow::saveSettings() is called. */
    delete centralWidget();

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
