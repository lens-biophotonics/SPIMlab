#include "mainwindow.h"

#include "camerapage.h"
#include "coboltwidget.h"
#include "filterswidget.h"
#include "settingswidget.h"
#include "spim.h"
#include "stagewidget.h"
#include "version.h"

#include <qtlab/core/logmanager.h>
#include <qtlab/hw/ni/natinst.h>
#include <qtlab/hw/pi/pidevice.h>
#include <qtlab/hw/serial/AA_MPDSnCxx.h>
#include <qtlab/hw/serial/cobolt.h>
#include <qtlab/hw/serial/filterwheel.h>
#include <qtlab/hw/serial/serialport.h>
#include <qtlab/widgets/logwidget.h>

#include <qwt_global.h>

#include <QApplication>
#include <QCloseEvent>
#include <QMenuBar>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QSettings>
#include <QTextStream>
#include <QToolBar>

static Logger *logger = getLogger("MainWindow");

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
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

MainWindow::~MainWindow() {}

void MainWindow::setupUi()
{
    connect(&spim(), &SPIM::error, this, [=](QString s) {
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
    connect(saveSettingsAction, &QAction::triggered, this, &MainWindow::saveSettings);

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

    CameraPage *centralWidget = new CameraPage(this);
    setCentralWidget(centralWidget);

    QWidget *cw;
    QBoxLayout *vLayout;

    vLayout = new QVBoxLayout();
    cw = new QWidget();
    QBoxLayout *laserLayout = new QVBoxLayout();
    for (int i = 0; i < SPIM_NCOBOLT; i++) {
        laserLayout->addWidget(new CoboltWidget(spim().getLaser(i)));
    }
    cw->setLayout(laserLayout);
    cw->setWindowTitle("Lasers");
    closableWidgets << cw;

    vLayout = new QVBoxLayout();
    cw = new QWidget();
    vLayout->addWidget(new FiltersWidget());
    cw->setLayout(vLayout);
    cw->setWindowTitle("Filters");
    closableWidgets << cw;

    vLayout = new QVBoxLayout();
    cw = new QWidget();
    vLayout->addWidget(new StageWidget());
    cw->setLayout(vLayout);
    cw->setWindowTitle("Stages");
    closableWidgets << cw;

    vLayout = new QVBoxLayout();
    cw = new QWidget();
    vLayout->addWidget(new SettingsWidget());
    cw->setLayout(vLayout);
    cw->setWindowTitle("Settings");
    closableWidgets << cw;

    vLayout = new QVBoxLayout();
    cw = new QWidget();
    QWidget *w = new LogWidget();
    w->setMinimumSize(800, 500);
    vLayout->addWidget(w);
    cw->setLayout(vLayout);
    cw->setWindowTitle("Messages");
    closableWidgets << cw;

    QToolBar *toolbar = addToolBar("Main");
    toolbar->setObjectName("MainToolbar");
    toolbar->setMovable(false);

    QMap<QWidget *, QAction *> actionMap;
    for (QWidget *w : closableWidgets) {
        QAction *action = toolbar->addAction(w->windowTitle(), this, [=]() {
            w->show();
            w->activateWindow();
        });
        actionMap[w] = action;
        w->adjustSize();
    }
}

void MainWindow::saveSettings() const
{
    QSettings settings;

    settings.beginGroup("MainWindow");
    settings.setValue("mainWindowGeometry", saveGeometry());
    settings.setValue("mainWindowState", saveState());

    for (QWidget *w : closableWidgets) {
        settings.setValue(w->windowTitle() + "_geometry", w->saveGeometry());
        settings.setValue(w->windowTitle() + "_shown", w->isVisible());
    }

    settings.endGroup();
}

void MainWindow::loadSettings()
{
    QString group;

    QSettings settings;

    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    restoreState(settings.value("mainWindowState").toByteArray());
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
#ifndef DEMO_MODE
    QMessageBox::StandardButton ret
        = QMessageBox::question(this,
                                QString("Closing %1").arg(PROGRAM_NAME),
                                QString("Are you sure you want to close %1?").arg(PROGRAM_NAME),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No);

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

    QMetaObject::invokeMethod(&spim(), "uninitialize", Qt::BlockingQueuedConnection);
    qDeleteAll(closableWidgets);
    QMainWindow::closeEvent(e);
}

void MainWindow::restoreWidgets()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    for (QWidget *w : closableWidgets) {
        w->restoreGeometry(settings.value(w->windowTitle() + "_geometry").toByteArray());
        if (settings.value(w->windowTitle() + "_shown").toBool()) {
            w->show();
        }
    }
    settings.endGroup();
}
