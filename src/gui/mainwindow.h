#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>

#include "version.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_aboutAction_triggered();
    void on_quitAction_triggered();

private:
    void setupUi();
    void setupDevices();
    void closeEvent(QCloseEvent *e = NULL);
};

#endif // MAINWINDOW_H
