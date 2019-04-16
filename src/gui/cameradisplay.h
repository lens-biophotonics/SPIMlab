#ifndef CAMERADISPLAY_H
#define CAMERADISPLAY_H

#include <QWidget>
#include <QThread>

class OrcaFlash;
class CameraPlot;


class CameraDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit CameraDisplay(OrcaFlash *camera, QWidget *parent = nullptr);
    virtual ~CameraDisplay();

signals:

public slots:
    void replot();

private:
    CameraPlot *plot;
    QVector<double> vec;
    OrcaFlash *orca;

    void setupUi();
};


class DisplayWorker : public QThread
{
    Q_OBJECT
public:
    DisplayWorker(OrcaFlash *orca, double *data, QObject *parent = nullptr);
    virtual ~DisplayWorker();

signals:
    void newImage();

private slots:
    void updateImage();

private:
    QTimer *timer;
    OrcaFlash *orca;
    double *buf;
    uint16_t *mybuf;
};

#endif // CAMERADISPLAY_H
