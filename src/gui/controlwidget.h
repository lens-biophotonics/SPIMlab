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

private:
    void setupUi();
};

#endif // CONTROLWIDGET_H
