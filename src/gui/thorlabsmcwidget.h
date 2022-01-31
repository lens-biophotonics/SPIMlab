#ifndef THORLABSMCWIDGET_H
#define THORLABSMCWIDGET_H

#include <qtlab/hw/thorlabs-mc/motorcontroller.h>

#include <QWidget>

using namespace QtLab::hw::Thorlabs;

class ThorlabsMCWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ThorlabsMCWidget(QtLab::hw::Thorlabs::MotorController *mc, QWidget *parent = nullptr);

signals:

private slots:
    void refreshValues();

private:
    void setupUI();

    QtLab::hw::Thorlabs::MotorController *mc;
};

#endif // THORLABSMCWIDGET_H
