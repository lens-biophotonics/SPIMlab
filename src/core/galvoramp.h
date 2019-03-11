#ifndef GALVORAMP_H
#define GALVORAMP_H

#include <QVector>

#include "niabstracttask.h"

#define GALVORAMP_N_OF_PARAMS 4
#define GALVORAMP_OFFSET_IDX 0
#define GALVORAMP_AMPLITUDE_IDX 1
#define GALVORAMP_PHASE_IDX 2
#define GALVORAMP_RAMP_FRACTION_IDX 3

class GalvoRamp : public NIAbstractTask
{
public:
    GalvoRamp(QObject *parent = nullptr);

    QString getPhysicalChannels() const;
    void setPhysicalChannels(const QString &channels);
    void setPhysicalChannels(const QStringList &channels);

    void setTriggerSource(const QString &source);

    void setWaveformAmplitude(const int channelNumber, const double val);
    void setWaveformOffset(const int channelNumber, const double val);
    void setWaveformPhase(const int channelNumber, const double val);
    void setWaveformRampFraction(const int channelNumber, const double val);

    double getWaveformAmplitude(const int channelNumber) const;
    double getWaveformOffset(const int channelNumber) const;
    double getWaveformPhase(const int channelNumber) const;
    double getWaveformRampFraction(const int channelNumber) const;

    QVector<double> getWaveformParams() const;
    void setWaveformParams(const QVector<double> &values);

    int nOfChannels();

    int getNRamp(const int channelNumber) const;
    void setNRamp(const int channelNumber, const int value);

protected:
    virtual void initializeTask_impl();

private:
    QVector<double> waveformParams;

    QStringList physicalChannels;
    QString triggerTerm;
    QString triggerSource;
    QVector<double> waveform;

    QVector<int> nRamp;

    void write();
    void computeWaveform();
    void appendToWaveform(double offset,
                          const double amplitude, const int nRamp,
                          const int delay);
    void setWaveformParam(const int channelNumber,
                          const int paramID, const double val);
};

#endif // GALVORAMP_H
