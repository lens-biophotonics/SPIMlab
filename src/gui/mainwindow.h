#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMainWindow>

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
    Q_INVOKABLE void restoreWidgets();
    QList<QWidget *> closableWidgets;
};

#endif // MAINWINDOW_H
