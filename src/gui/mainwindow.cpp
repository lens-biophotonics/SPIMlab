#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QCloseEvent>
#include <QToolBar>
#include <QLabel>
#include <QStatusBar>
#include <QSettings>

#include "core/spim.h"
#include "core/statemachine.h"
#include "core/logmanager.h"

#include "mainwindow.h"
#include "centralwidget.h"

static Logger *logger = getLogger("MainWindow");

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUi();

    loadSettings();

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

    setMinimumSize(1024, 768);

    QState *s;

    s = stateMachine().getState(STATE_UNINITIALIZED);
    s->assignProperty(statusLabel, "text", "Uninitialized");

    s = stateMachine().getState(STATE_READY);
    s->assignProperty(statusLabel, "text", "Ready");

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
}

void MainWindow::loadSettings()
{
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
