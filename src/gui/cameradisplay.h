#ifndef CAMERADISPLAY_H
#define CAMERADISPLAY_H

#include <QWidget>
#include <QThread>

#include "cameraplot.h"


class CameraDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit CameraDisplay(QWidget *parent = nullptr);
    virtual ~CameraDisplay();

signals:

public slots:
    void replot();

private:
    CameraPlot *plot;
    QThread *thread;
    QVector<double> vec;

    void setupUi();
};


class DisplayWorker : public QThread
{
    Q_OBJECT
public:
    DisplayWorker(double *data, QObject *parent = nullptr);
    virtual ~DisplayWorker();

signals:
    void newImage();

private slots:
    void updateImage();

private:
    QTimer *timer;
    double *buf;
};

#endif // CAMERADISPLAY_H
