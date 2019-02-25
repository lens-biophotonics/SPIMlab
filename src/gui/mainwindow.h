#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QMap>

#include <core/pidevice.h>
#include "version.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_aboutAction_triggered() const;

private:
    void setupUi();
    void saveSettings() const;
    void loadSettings();
    void closeEvent(QCloseEvent *e = nullptr);

    QMap<QString, PIDevice *> piSettingsPrefixMap;
};

#endif // MAINWINDOW_H
