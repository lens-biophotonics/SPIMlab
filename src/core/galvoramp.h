#ifndef GALVORAMP_H
#define GALVORAMP_H

#include <QVector>

#include "niabstracttask.h"

using namespace NI;

class GalvoRamp : public NIAbstractTask
{
public:
    GalvoRamp(QObject *parent = nullptr);

    QString getPhysicalChannel() const;
    void setPhysicalChannel(const QString &channel);

    void setTriggerSource(QString source);

    void setCameraParams(int nSamples, const int nRamp, const double rate);

    void setupWaveform(double offset, const double amplitude, const int delay);

protected:
    virtual void initializeTask_impl();

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

    void configureTiming();
    void write();
    void computeWaveform();
};

#endif // GALVORAMP_H
