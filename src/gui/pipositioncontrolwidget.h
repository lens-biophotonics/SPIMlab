#ifndef PIPOSITIONCONTROLWIDGET_H
#define PIPOSITIONCONTROLWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>

#include "core/pidevice.h"

class PIPositionControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PIPositionControlWidget(PIDevice *device,
                                     QWidget *parent = nullptr);

    void appendDummyRows(const int number);
signals:

public slots:

private slots:
    void updateUIConnect();
    void updateValues(const QString &axes, const QVector<double> &pos);

private:
    void setupUI();
    void clear();
    void appendRow(const int row, const QString &name);
    void move(const QString &name, const double pos);
    void moveRelative(const QString &name, const double pos);


    PIDevice *device;

    QGridLayout *grid;
    QMap<QChar, QLabel*> currentPosLabelMap;
};

#endif // PIPOSITIONCONTROLWIDGET_H
