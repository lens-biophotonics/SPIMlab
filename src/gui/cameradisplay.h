#ifndef CAMERADISPLAY_H
#define CAMERADISPLAY_H

#include <QWidget>
#include <QTimer>

#include "cameraplot.h"

class CameraDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit CameraDisplay(QWidget *parent = nullptr);
    virtual ~CameraDisplay();

signals:

public slots:

private:
    CameraPlot *plot;
    QTimer *timer;

    void setupUi();

private slots:
    void on_timer_timeout();
};

#endif // CAMERADISPLAY_H
