#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QCloseEvent>
#include <QToolBar>
#include <QPushButton>

#include "spimhub.h"

#include "mainwindow.h"
#include "logmanager.h"
#include "centralwidget.h"

static Logger *logger = LogManager::getInstance().getLogger("MainWindow");

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUi();

    setupDevices();

    QMetaObject::connectSlotsByName(this);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    QAction *quitAction = new QAction(this);
    quitAction->setText("&Quit");
    quitAction->setObjectName("quitAction");
    quitAction->setShortcut(QKeySequence("Ctrl+Q"));

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

    QPushButton *startCapturePushButton = new QPushButton("Start capture");
    startCapturePushButton->setObjectName("startCapturePushButton");
    QPushButton *stopCapturePushButton = new QPushButton("Stop capture");
    stopCapturePushButton->setObjectName("stopCapturePushButton");

    QToolBar *toolbar = addToolBar("Main toolbar");
    toolbar->addWidget(startCapturePushButton);
    toolbar->addWidget(stopCapturePushButton);

    setMinimumSize(1024, 768);
}

void MainWindow::setupDevices()
{
#ifdef WITH_HARDWARE
    init_dcam();
#endif

    OrcaFlash *orca = new OrcaFlash(this);
    orca->open(0);

    SPIMHub::getInstance().setCamera(orca);
}

void MainWindow::on_aboutAction_triggered()
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

void MainWindow::on_quitAction_triggered()
{
    closeEvent();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
#ifdef WITH_HARDWARE
    QMessageBox::StandardButton ret = QMessageBox::question(
        this, QString("Closing %1").arg(PROGRAM_NAME),
        QString("Are you sure you want to close %1?").arg(PROGRAM_NAME),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (ret != QMessageBox::Yes) {
        if (e)
            e->ignore();
        return;
    }

    uninit_dcam();
#else
    Q_UNUSED(e)
#endif
    qApp->quit();
}

void MainWindow::on_startCapturePushButton_clicked()
{
    SPIMHub::getInstance().startFreeRun();
}

void MainWindow::on_stopCapturePushButton_clicked()
{
    SPIMHub::getInstance().stop();
}
