#include <cmath>

#include "galvoramp.h"

using namespace NI;

#define CHANNEL_NAME "galvoRampAOChan"

GalvoRamp::GalvoRamp(QObject *parent) : NIAbstractTask(parent)
{
    setTaskName("galvoRampAO");
}

void GalvoRamp::setPhysicalChannels(const QString &channels)
{
    setPhysicalChannels(channels.split(":"));
}

void GalvoRamp::setPhysicalChannels(const QStringList &channels)
{
    physicalChannels = channels;
    clear();
    waveformParams.clear();
    waveformParams.fill(0, GALVORAMP_N_OF_PARAMS * nOfChannels());
    nRamp.clear();
    nRamp.fill(0, nOfChannels());
}

void GalvoRamp::setTriggerSource(const QString &source)
{
    if (triggerSource == source) {
        return;
    }
    triggerSource = source;
    clear();
}

void GalvoRamp::setWaveformAmplitude(const int channelNumber, const double val)
{
    setWaveformParam(channelNumber, GALVORAMP_AMPLITUDE_IDX, val);
}

void GalvoRamp::setWaveformOffset(const int channelNumber, const double val)
{
    setWaveformParam(channelNumber, GALVORAMP_OFFSET_IDX, val);
}

void GalvoRamp::setWaveformPhase(const int channelNumber, const double val)
{
    setWaveformParam(channelNumber, GALVORAMP_PHASE_IDX, val);
}

void GalvoRamp::setWaveformRampFraction(const int channelNumber, const double val)
{
    setWaveformParam(channelNumber, GALVORAMP_RAMP_FRACTION_IDX, val);
    nRamp[channelNumber] = round(nSamples * val);
}

double GalvoRamp::getWaveformAmplitude(const int channelNumber) const
{
    return waveformParams[
        channelNumber * GALVORAMP_N_OF_PARAMS + GALVORAMP_AMPLITUDE_IDX];
}

double GalvoRamp::getWaveformOffset(const int channelNumber) const
{
    return waveformParams[
        channelNumber * GALVORAMP_N_OF_PARAMS + GALVORAMP_OFFSET_IDX];
}

double GalvoRamp::getWaveformPhase(const int channelNumber) const
{
    return waveformParams[
        channelNumber * GALVORAMP_N_OF_PARAMS + GALVORAMP_PHASE_IDX];
}

double GalvoRamp::getWaveformRampFraction(const int channelNumber) const
{
    return 1. * nSamples / nRamp.at(channelNumber);
}

QVector<double> GalvoRamp::getWaveformParams() const
{
    return waveformParams;
}

void GalvoRamp::setWaveformParams(const QVector<double> &values)
{
    waveformParams = values;
}

int GalvoRamp::nOfChannels()
{
    return physicalChannels.count();
}

void GalvoRamp::initializeTask_impl()
{
    DAQmxErrChk(
        DAQmxCreateAOVoltageChan(
            task,
            getPhysicalChannels().toLatin1(),
            CHANNEL_NAME,
            -10.0,  // minVal
            10.0,  // maxVal
            DAQmx_Val_Volts,  // units
            nullptr // customScaleName
            )
        );
    DAQmxErrChk(
        DAQmxCfgDigEdgeStartTrig(
            task,
            triggerSource.toLatin1(),
            DAQmx_Val_Rising
            )
        );
    DAQmxErrChk(DAQmxSetStartTrigRetriggerable(task, true));

    configureSampleClockTiming("", DAQmx_Val_Rising, DAQmx_Val_FiniteSamps);
    computeWaveform();
    write();
}

int GalvoRamp::getNRamp(const int channelNumber) const
{
    return nRamp.at(channelNumber);
}

void GalvoRamp::setNRamp(const int channelNumber, const int value)
{
    nRamp[channelNumber] = value;
}

QString GalvoRamp::getPhysicalChannels() const
{
    return physicalChannels.join(":");
}

void GalvoRamp::write()
{
    DAQmxErrChk(
        DAQmxWriteAnalogF64(
            task,
            waveform.size() / nOfChannels(),
            false,  // autostart
            10,  // timeout in seconds
            DAQmx_Val_GroupByChannel,
            waveform.data(),
            nullptr,
            nullptr  // reserved
            )
        );
}

void GalvoRamp::computeWaveform()
{
    waveform.clear();
    waveform.reserve(static_cast<int>(nSamples) * nOfChannels());
    for (int i = 0; i < nOfChannels(); ++i) {
        int delay = static_cast<int>(
            waveformParams[GALVORAMP_N_OF_PARAMS * i + GALVORAMP_PHASE_IDX]
            * nSamples);
        ;
        appendToWaveform(
            waveformParams[GALVORAMP_N_OF_PARAMS * i + GALVORAMP_OFFSET_IDX],
            waveformParams[GALVORAMP_N_OF_PARAMS * i + GALVORAMP_AMPLITUDE_IDX],
            nRamp.at(i),
            delay);
    }
}

void GalvoRamp::appendToWaveform(
    double offset, const double amplitude, const int nRamp, const int delay)
{
    int nSamples = static_cast<int>(this->nSamples);
    QVector<double> temp(static_cast<int>(nSamples), 0);
    double halfAmplitude = 0.5 * amplitude;
    int i = 0;
    for (; i < nRamp; ++i)
        temp[i] = offset - halfAmplitude + amplitude * i / nRamp;
    int nRamp2 = nSamples - nRamp;
    for (; i < nSamples; ++i)
        temp[i] = offset + halfAmplitude - amplitude * (i - nRamp) / nRamp2;

    if (delay == 0 || delay == temp.size()) {
        waveform << temp;
        return;
    }

    if (delay > 0) {
        waveform << temp.mid(nSamples - delay - 1, delay);
        waveform << temp.mid(0, nSamples - delay);
    } else {
        waveform << temp.mid(delay, nSamples + delay);
        waveform << temp.mid(0, -delay);
    }
}

void GalvoRamp::setWaveformParam(
    const int channelNumber, const int paramID, const double val)
{
    int idx = GALVORAMP_N_OF_PARAMS * channelNumber + paramID;
    waveformParams[idx] = val;

    if (isInitialized()) {
        computeWaveform();
        write();
    }
}
