#ifndef PIPOSITIONCONTROLWIDGET_H
#define PIPOSITIONCONTROLWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>

enum SPIM_PI_DEVICES : int;

class PIDevice;

class PIPositionControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PIPositionControlWidget(QWidget *parent = nullptr);
    void appendRow(SPIM_PI_DEVICES d_enum, const QString &axis,
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
