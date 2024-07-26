#ifndef CAMERADELAYWIDGET_H
#define CAMERADELAYWIDGET_H

#include <QDoubleSpinBox>
#include <QWidget>

class CameraDelayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CameraDelayWidget(QWidget *parent = nullptr);

private:
    void setupUI();
};

#endif // CAMERADELAYWIDGET_H
