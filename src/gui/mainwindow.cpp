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

#include "mainwindow.h"
#include "centralwidget.h"

static Logger *logger = getLogger("MainWindow");

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    piSettingsPrefixMap["X_AXIS."] = spim().piDevice(SPIM::PI_DEVICE_X_AXIS);
    piSettingsPrefixMap["Y_AXIS."] = spim().piDevice(SPIM::PI_DEVICE_Y_AXIS);
    piSettingsPrefixMap["Z_AXIS."] = spim().piDevice(SPIM::PI_DEVICE_Z_AXIS);
    piSettingsPrefixMap["LO_AXIS."] =
        spim().piDevice(SPIM::PI_DEVICE_LEFT_OBJ_AXIS);
    piSettingsPrefixMap["RO_AXIS."] =
        spim().piDevice(SPIM::PI_DEVICE_RIGHT_OBJ_AXIS);

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
    QSettings settings;

    settings.beginGroup("MainWindow");
    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());
    settings.endGroup();

    settings.beginGroup("SPIM");

    QMapIterator<QString, PIDevice *> i(piSettingsPrefixMap);
    while (i.hasNext()) {
        i.next();
        PIDevice *dev = i.value();
        QString prefix = i.key();
        settings.setValue(prefix + "portName", dev->getPortName());
        settings.setValue(prefix + "baud", dev->getBaud());
        settings.setValue(prefix + "deviceNumber", dev->getDeviceNumber());
        if (!dev->getPortName().isEmpty()) {
            QSerialPortInfo info(dev->getPortName());
            settings.setValue(prefix + "serialNumber", info.serialNumber());
        }
    }

    settings.setValue("galvoRamp.physicalChannel",
                      spim().getGalvoRamp()->getPhysicalChannels());
    QList<QVariant> waveformParams;
    foreach (QVariant variant, spim().getGalvoRamp()->getWaveformParams()) {
        waveformParams.append(variant.toDouble());
    }
    settings.setValue("galvoRamp.waveformParams", waveformParams);

    for (int i = 0; i < SPIM_NCAMS; ++i) {
        settings.setValue(QString("cameraTrigger.physicalChannel_%1").arg(i),
                          spim().getCameraTrigger()->getPhysicalChannel(i));
        settings.setValue(QString("cameraTrigger.term_%1").arg(i),
                          spim().getCameraTrigger()->getTerm(i));
    }

    settings.setValue("exposureTime", spim().getExposureTime());

    settings.endGroup();
}

void MainWindow::loadSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    restoreState(settings.value("mainWindowState").toByteArray());
    settings.endGroup();

    settings.beginGroup("SPIM");

    QMapIterator<QString, PIDevice *> i(piSettingsPrefixMap);
    while (i.hasNext()) {
        i.next();
        PIDevice *dev = i.value();
        QString prefix = i.key();
        dev->setBaud(settings.value(prefix + "baud", "").toInt());
        dev->setDeviceNumber(settings.value(prefix + "deviceNumber", "").toInt());

        QString sn = settings.value(prefix + "serialNumber").toString();

        if (!sn.isEmpty()) {
            QSerialPortInfo info = SerialPort::findPortFromSerialNumber(sn);
            if (!info.portName().isEmpty()) {
                dev->setPortName(info.portName());
            }
        }
        else {
            dev->setPortName(settings.value(prefix + "portName").toString());
        }
    }

    QStringList physicalChannels;
    QStringList terms;

    GalvoRamp *gr = spim().getGalvoRamp();
    gr->setPhysicalChannels(
        settings.value("galvoRamp.physicalChannel",
                       "Dev1/ao0:Dev1/ao1").toString());
    QList<QVariant> wafeformParams =
        settings.value("galvoRamp.waveformParams",
                       QList<QVariant>({0.2, 2., 0., 100., 0.2, 2., 0.5, 100.})).toList();

    QVector<double> wp;

    for (int i = 0; i < wafeformParams.count(); i++) {
        wp << wafeformParams.at(i).toDouble();
    }

    gr->setWaveformParams(wp);

    for (int i = 0; i < SPIM_NCAMS; ++i) {
        physicalChannels << settings.value(
            QString("cameraTrigger.physicalChannel_%1").arg(i),
            QString("Dev1/ctr%1").arg(i)).toString();

        terms << settings.value(QString("cameraTrigger.term_%1").arg(i),
                                QString("/Dev1/PFI%1").arg(i)).toString();
    }

    CameraTrigger *ct = spim().getCameraTrigger();
    ct->setPhysicalChannels(physicalChannels);
    ct->setTerms(terms);

    spim().setExposureTime(settings.value("exposureTime", 0.1).toDouble());

    settings.endGroup();
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
    foreach (PIDevice * dev, spim().piDevices()) {
        if (!dev->isConnected()) {
            continue;
        }
        dev->getCurrentPosition();
    }
}
