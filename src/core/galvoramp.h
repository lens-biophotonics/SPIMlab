#ifndef GALVORAMP_H
#define GALVORAMP_H

#include <QVector>

#include "niabstracttask.h"

using namespace NI;

class GalvoRamp : public NIAbstractTask
{
public:
    GalvoRamp(QObject *parent);
    ~GalvoRamp();

    void setPhysicalChannels(QString channel);
    void setTriggerSource(QString source);

    void setCameraParams(int nSamples, int nRamp, double rate);

    void setupWaveform(double offset, double amplitude, int delay);

protected:
    virtual bool initializeTask_impl();

private:
    double rate;
    int nSamples;
    int nRamp;

    QString physicalChannel;
    QString triggerTerm;
    QString triggerSource;
    QVector<double> waveform;

    double offset;
    double amplitude;
    int delay;

    bool configureTiming();
    bool write();
    void computeWaveform();
};

#endif // GALVORAMP_H
