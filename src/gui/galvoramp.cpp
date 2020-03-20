#include <cmath>

#include "galvoramp.h"
#include <qtlab/core/logger.h>

static Logger *logger = getLogger("GalvorampTrigger");

using namespace NI;

#define CHANNEL_NAME "galvoRampAOChan"

GalvoRamp::GalvoRamp(QObject *parent) : NIAbstractTask(parent)
{
    setTaskName("galvoRampAO");
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

int GalvoRamp::nOfChannels()
{
    return physicalChannels.count();
}

void GalvoRamp::initializeTask_impl()
{
    DAQmxErrChk(
        DAQmxCreateAOVoltageChan(
            task,
            getPhysicalChannels().join(":").toLatin1(),
            CHANNEL_NAME,
            -10.0,  // minVal
            10.0,  // maxVal
            DAQmx_Val_Volts,  // units
            nullptr // customScaleName
            )
        );

    configureTriggering();
    configureSampleClockTiming("", DAQmx_Val_Rising, DAQmx_Val_ContSamps);
    computeWaveform();
    write();
}

void GalvoRamp::setPhysicalChannels_impl()
{
    waveformParams.clear();
    waveformParams.fill(0, GALVORAMP_N_OF_PARAMS * nOfChannels());
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
    double delay = static_cast<double>(nSamples) / nOfChannels() / getSampleRate(); // hardcoded delay between waveforms
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
    int nSamples = static_cast<int>(this->nSamples);
    int nRamp = static_cast<int>(floor(nSamples * fraction));
    int nDelay = static_cast<int>(round(delay * getSampleRate()));
    double halfAmplitude = 0.5 * amplitude;
    QVector<double> temp(static_cast<int>(nSamples), 0);

    int i = 0;
    for (; i < nRamp; ++i)
        temp[i] = offset - halfAmplitude + amplitude * i / nRamp;
    int nRamp2 = nSamples - nRamp;
    for (; i < nSamples; ++i)
        temp[i] = offset + halfAmplitude - amplitude * (i - nRamp) / nRamp2;

    if (nDelay == 0 || nDelay == temp.size()) {
        waveform << temp;
    }
    else if (nDelay > 0) {
        waveform << temp.mid(nSamples - nDelay);
        waveform << temp.mid(0, nSamples - nDelay);
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
