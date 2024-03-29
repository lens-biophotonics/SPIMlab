#ifndef GALVORAMP_H
#define GALVORAMP_H

#include <qtlab/hw/ni/nitask.h>

#include <QVector>

#define GALVORAMP_N_OF_PARAMS 4
#define GALVORAMP_OFFSET_IDX 0
#define GALVORAMP_AMPLITUDE_IDX 1
#define GALVORAMP_DELAY_IDX 2
#define GALVORAMP_FRACTION_IDX 3

class GalvoRamp : public NITask
{
public:
    GalvoRamp(QObject *parent = nullptr);

    void setWaveformAmplitude(const int channelNumber, const double val);
    void setWaveformOffset(const int channelNumber, const double val);
    void setWaveformDelay(const int channelNumber, const double val);
    void setWaveformRampFraction(const int channelNumber, const double val);

    double getWaveformAmplitude(const int channelNumber) const;
    double getWaveformOffset(const int channelNumber) const;
    double getWaveformDelay(const int channelNumber) const;
    double getWaveformRampFraction(const int channelNumber) const;

    QVector<double> getWaveformParams() const;
    void setWaveformParams(const QVector<double> &values);
    void resetWaveFormParams(const int nOfChannels);

    void updateWaveform();

protected:
    virtual void initializeTask_impl() override;

private:
    QVector<double> waveformParams;
    QVector<double> waveform;

    void write();
    void computeWaveform();
    void appendToWaveform(double offset,
                          const double amplitude,
                          const double fraction,
                          const double delay);
    void setWaveformParam(const int channelNumber, const int paramID, const double val);
};

#endif // GALVORAMP_H
