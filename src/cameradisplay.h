#ifndef CAMERADISPLAY_H
#define CAMERADISPLAY_H

#include <QWidget>
#include <QTimer>

#include <boost/shared_ptr.hpp>
#include <boost/signals2/deconstruct.hpp>

#include "spimhub.h"
#include "cameraplot.h"

namespace bs2 = boost::signals2;

class CameraDisplay : public QWidget
{
    Q_OBJECT
public:
    template<typename T> friend
    void adl_postconstruct(const boost::shared_ptr<T> &sp, CameraDisplay *)
    {
        SPIMHub::getInstance().freeRunStarted.connect(
            simpleSignal_t::slot_type(
                &CameraDisplay::startRefreshTimer, sp.get()).track(sp));
        SPIMHub::getInstance().stopped.connect(
            simpleSignal_t::slot_type(
                &CameraDisplay::stopRefreshTimer, sp.get()).track(sp));
    }

    virtual ~CameraDisplay();

    void startRefreshTimer();
    void stopRefreshTimer();

signals:

public slots:

private:
    CameraPlot *plot;
    QTimer *timer;
    uint16_t *buf;
    QVector<double> vec;

    friend class bs2::deconstruct_access;
    explicit CameraDisplay(QWidget *parent = nullptr);

    void setupUi();

private slots:
    void on_timer_timeout();
};

#endif // CAMERADISPLAY_H
