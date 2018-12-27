#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>

#include "version.h"
#include "orcaflash.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_aboutAction_triggered();
    void on_quitAction_triggered();
    void on_startFreeRunPushButton_clicked();
    void on_stopFreeRunPushButton_clicked();

private:
    void setupUi();
    void setupDevices();
    void closeEvent(QCloseEvent *e = NULL);

    OrcaFlash *orca;
};

#endif // MAINWINDOW_H
