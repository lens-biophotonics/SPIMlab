#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>
#include <QTabWidget>

#include "cameraplot.h"
class CentralWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CentralWidget(QWidget *parent = 0);

signals:

public slots:

private:
    void setupUi();

    CameraPlot *cameraDisplay;
};

#endif // CENTRALWIDGET_H
