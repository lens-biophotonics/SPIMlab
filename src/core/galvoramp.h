#ifndef GALVORAMP_H
#define GALVORAMP_H

#include <QVector>

#include "niabstracttask.h"

class GalvoRamp : public NIAbstractTask
{
public:
    GalvoRamp(QObject *parent = nullptr);

    QString getPhysicalChannels() const;
    void setPhysicalChannels(const QString &channels);
    void setPhysicalChannels(const QStringList &channels);

    void setTriggerSource(const QString &source);

    void setCameraParams(int nSamples, const int nRamp, const double rate);

    void setWaveformParams(double offset, const double amplitude, const int delay);
    void setWaveformParams(const QList<QVariant> &list);
    QList<QVariant> getWaveformParams() const;

protected:
    virtual void initializeTask_impl();

private:
    double rate;
    int nSamples;
    int nRamp;

    QString physicalChannels;
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
