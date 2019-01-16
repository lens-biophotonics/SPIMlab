#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>
#include <QTabWidget>

class CentralWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CentralWidget(QWidget *parent = nullptr);
    virtual ~CentralWidget();

signals:

public slots:

private:
    void setupUi();
    void saveSettings();
    void loadSettings();

    QTabWidget *tabWidget;
};

#endif // CENTRALWIDGET_H
