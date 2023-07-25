#ifndef GALVOSWIDGET_H
#define GALVOSWIDGET_H

#include <QDoubleSpinBox>
#include <QWidget>

class GalvosWidget: public QWidget
{
    Q_OBJECT
public:
    explicit GalvosWidget(QWidget *parent = nullptr);

private:
    void setupUI();
};

#endif // GALVOSWIDGET_H
