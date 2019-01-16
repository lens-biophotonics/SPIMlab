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

    void createWaveform(int nSamples, int rampSamples, double offset,
                        double amplitude, int delay, double rate);

protected:
    virtual bool initializeTask();

private:
    double rate;
    QString physicalChannel;
    QString triggerTerm;
    QString triggerSource;
    QVector<double> waveform;
};

#endif // GALVORAMP_H
