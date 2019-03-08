#ifndef PIPOSITIONCONTROLWIDGET_H
#define PIPOSITIONCONTROLWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>

#include "core/pidevice.h"
#include "customspinbox.h"

class PIPositionControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PIPositionControlWidget(QWidget *parent = nullptr);
    void appendRow(PIDevice *device, const QString &axis,
                   const QString &axisName);

    void setTitle(const QString &title);

private:
    void setupUI();

    int row = 0;
    PIDevice *device;

    QGridLayout *grid;
    QGroupBox *gb;
};

#endif // PIPOSITIONCONTROLWIDGET_H
