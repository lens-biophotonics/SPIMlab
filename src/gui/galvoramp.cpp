#include <cmath>

#include "galvoramp.h"
#include <qtlab/core/logger.h>

static Logger *logger = getLogger("GalvorampTrigger");

using namespace NI;

#define CHANNEL_NAME "galvoRampAOChan"

GalvoRamp::GalvoRamp(QObject *parent) : NITask("galvoRampAO", parent)
{
}

void GalvoRamp::setWaveformAmplitude(const int channelNumber, const double val)
{
    setWaveformParam(channelNumber, GALVORAMP_AMPLITUDE_IDX, val);
}

void GalvoRamp::setWaveformOffset(const int channelNumber, const double val)
{
    setWaveformParam(channelNumber, GALVORAMP_OFFSET_IDX, val);
}

void GalvoRamp::setWaveformDelay(const int channelNumber, const double val)
{
    setWaveformParam(channelNumber, GALVORAMP_DELAY_IDX, val);
}

void GalvoRamp::setWaveformRampFraction(const int channelNumber, const double val)
{
    setWaveformParam(channelNumber, GALVORAMP_FRACTION_IDX, val);
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

double GalvoRamp::getWaveformDelay(const int channelNumber) const
{
    return waveformParams[
        channelNumber * GALVORAMP_N_OF_PARAMS + GALVORAMP_DELAY_IDX];
}

double GalvoRamp::getWaveformRampFraction(const int channelNumber) const
{
    return waveformParams[
        channelNumber * GALVORAMP_N_OF_PARAMS + GALVORAMP_FRACTION_IDX];
}

QVector<double> GalvoRamp::getWaveformParams() const
{
    return waveformParams;
}

void GalvoRamp::setWaveformParams(const QVector<double> &values)
{
    waveformParams = values;
}

void GalvoRamp::updateWaveform()
{
    if (isInitialized() && !isTaskDone()) {
        computeWaveform();
        write();
    }
}

void GalvoRamp::initializeTask_impl()
{
    NITask::initializeTask_impl();

    computeWaveform();
    write();
}

void GalvoRamp::configureChannels_impl()
{
    createAOVoltageChan(
        physicalChannels.join(":").toLatin1(),
        CHANNEL_NAME,
        -10.0, 10.0, VoltUnits_Volts,
        nullptr);
}

void GalvoRamp::configureTriggering_impl()
{
    cfgDigEdgeStartTrig(triggerTerm.toLatin1(), Edge_Rising);
}

void GalvoRamp::configureTiming_impl()
{
    cfgSampClkTiming(
        sampClkTimingSource.isEmpty() ? nullptr : sampClkTimingSource.toLatin1(),
        sampleRate,
        Edge_Rising,
        SampMode_ContSamps,
        sampsPerChan);
}

void GalvoRamp::resetWaveFormParams(const int nOfChannels)
{
    waveformParams.clear();
    waveformParams.fill(0, GALVORAMP_N_OF_PARAMS * nOfChannels);
}

void GalvoRamp::write()
{
    writeAnalogF64(
        waveform.size() / nOfChannels(),
        false,
        10,
        DataLayout_GroupByChannel,
        waveform.data(),
        nullptr);
}

void GalvoRamp::computeWaveform()
{
    waveform.clear();
    waveform.reserve(static_cast<int>(sampsPerChan) * nOfChannels());
    double delay = static_cast<double>(sampsPerChan) / nOfChannels() / getSampleRate(); // hardcoded delay between waveforms
    for (int i = 0; i < nOfChannels(); ++i) {
        appendToWaveform(
            waveformParams[GALVORAMP_N_OF_PARAMS * i + GALVORAMP_OFFSET_IDX],
            waveformParams[GALVORAMP_N_OF_PARAMS * i + GALVORAMP_AMPLITUDE_IDX],
            waveformParams[GALVORAMP_N_OF_PARAMS * i + GALVORAMP_FRACTION_IDX],
            waveformParams[GALVORAMP_N_OF_PARAMS * i + GALVORAMP_DELAY_IDX] + delay * i);
    }
}

void GalvoRamp::appendToWaveform(
    double offset, const double amplitude, const double fraction, const double delay)
{
    int sampsPerChan = static_cast<int>(this->sampsPerChan);
    int nRamp = static_cast<int>(floor(sampsPerChan * fraction));
    int nDelay = static_cast<int>(round(delay * getSampleRate()));
    double halfAmplitude = 0.5 * amplitude;
    QVector<double> temp(static_cast<int>(sampsPerChan), 0);

    int i = 0;
    for (; i < nRamp; ++i)
        temp[i] = offset - halfAmplitude + amplitude * i / nRamp;
    int nRamp2 = sampsPerChan - nRamp;
    for (; i < sampsPerChan; ++i)
        temp[i] = offset + halfAmplitude - amplitude * (i - nRamp) / nRamp2;

    if (nDelay == 0 || nDelay == temp.size()) {
        waveform << temp;
    }
    else if (nDelay > 0) {
        waveform << temp.mid(sampsPerChan - nDelay);
        waveform << temp.mid(0, sampsPerChan - nDelay);
    } else {
        waveform << temp.mid(-nDelay);
        waveform << temp.mid(0, -nDelay);
    }
}

void GalvoRamp::setWaveformParam(
    const int channelNumber, const int paramID, const double val)
{
    int idx = GALVORAMP_N_OF_PARAMS * channelNumber + paramID;
    waveformParams[idx] = val;
}
