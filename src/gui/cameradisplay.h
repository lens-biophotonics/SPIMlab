#ifndef CAMERADISPLAY_H
#define CAMERADISPLAY_H

#include <QWidget>
#include <QThread>
#include <QMenu>

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

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);

private:
    CameraPlot *plot;
    QVector<double> vec;
    OrcaFlash *orca;
    QMenu *menu;

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

protected:
    virtual void run();

private:
    OrcaFlash *orca;
    double *buf;
    uint16_t *mybuf;
    bool running;
};

#endif // CAMERADISPLAY_H
