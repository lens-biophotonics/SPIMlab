#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H

#include <QWidget>

class ControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ControlWidget(QWidget *parent = nullptr);

signals:

public slots:

private slots:
    void on_initPushButton_clicked();
    void on_startCapturePushButton_clicked();
    void on_stopCapturePushButton_clicked();

private:
    void setupUi();
};

#endif // CONTROLWIDGET_H
