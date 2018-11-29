#ifndef GALVORAMP_H
#define GALVORAMP_H

#include <QVector>

#include "nidevice.h"

using namespace NI;

class GalvoRamp : public NIDevice
{
public:
    GalvoRamp();
    ~GalvoRamp();

    bool initializeTasks(QString physicalChannel, QString triggerSource);
    void createWaveform(uint nSamples, uint rampSamples, double offset,
                        double amplitude, int delay, double rate);

    bool start();
    bool stop();

private:
    TaskHandle task;
    double rate;
    QString physicalChannel, triggerTerm;
    QVector<double> waveform;

    bool clearTasks();
};

#endif // GALVORAMP_H
