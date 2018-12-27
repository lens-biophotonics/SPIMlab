#ifndef CAMERADISPLAY_H
#define CAMERADISPLAY_H

#include <QWidget>
#include <QTimer>

#include "cameraplot.h"
#include "orcaflash.h"

class CameraDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit CameraDisplay(QWidget *parent = nullptr);
    virtual ~CameraDisplay();

    void startRefreshTimer(int msec = 100);
    void stopRefreshTimer();

    void setCamera(OrcaFlash *camera);

signals:

public slots:

private:
    CameraPlot *plot;
    OrcaFlash *orca;
    QTimer *timer;
    uint16_t *buf;
    QVector<double> vec;


    void setupUi();
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_timer_timeout();
};

#endif // CAMERADISPLAY_H
