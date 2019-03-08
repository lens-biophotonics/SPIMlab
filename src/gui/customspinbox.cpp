#include <QLineEdit>

#include "customspinbox.h"


SpinBox::SpinBox(QWidget *parent) : QSpinBox (parent)
{
    connect(lineEdit(), &QLineEdit::returnPressed, [ = ](){
        emit returnPressed();
    });
}

DoubleSpinBox::DoubleSpinBox(QWidget *parent) : QDoubleSpinBox (parent)
{
    connect(lineEdit(), &QLineEdit::returnPressed, [ = ](){
        emit returnPressed();
    });
}
