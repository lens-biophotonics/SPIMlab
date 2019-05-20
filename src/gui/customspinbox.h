#ifndef CUSTOMSPINBOX_H
#define CUSTOMSPINBOX_H

#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>

class SpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit SpinBox(QWidget *parent = nullptr);

signals:
    void returnPressed(int value);
};

class DoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit DoubleSpinBox(QWidget *parent = nullptr);

signals:
    void returnPressed(double value);
};


#endif // CUSTOMSPINBOX_H
