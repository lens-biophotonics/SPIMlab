#include <QVariant>

#include "galvoramp.h"

using namespace NI;

#define CHANNEL_NAME "galvoRampAOChan"

GalvoRamp::GalvoRamp(QObject *parent) : NIAbstractTask(parent)
{
    setTaskName("galvoRampAO");
}

void GalvoRamp::setPhysicalChannels(const QString &channels)
{
    physicalChannels = channels;
    clear();
}

void GalvoRamp::setPhysicalChannels(const QStringList &channels)
{
    setPhysicalChannels(channels.join(":"));
}

void GalvoRamp::setTriggerSource(const QString &source)
{
    if (triggerSource == source) {
        return;
    }
    triggerSource = source;
    clear();
}

void GalvoRamp::setCameraParams(
    const int nSamples, const int nRamp, const double rate)
{
    this->nSamples = nSamples;
    this->nRamp = nRamp;
    this->rate = rate;

    if (isInitialized()) {
        computeWaveform();
        configureTiming();
        write();
    }
}

void GalvoRamp::setWaveformParams(
    const double offset, const double amplitude, const int delay)
{
    this->offset = offset;
    this->amplitude = amplitude;
    this->delay = delay;

    if (isInitialized()) {
        computeWaveform();
        write();
    }
}

void GalvoRamp::setWaveformParams(const QList<QVariant> &list)
{
    setWaveformParams(list.at(0).toDouble(),
                      list.at(1).toDouble(),
                      list.at(2).toInt());
}

QList<QVariant> GalvoRamp::getWaveformParams() const
{
    QList<QVariant> list;
    list.append(QVariant(offset));
    list.append(QVariant(amplitude));
    list.append(QVariant(delay));

    return list;
}

void GalvoRamp::initializeTask_impl()
{
    DAQmxErrChk(
        DAQmxCreateAOVoltageChan(
            task,
            physicalChannels.toLatin1(),
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

    computeWaveform();
    configureTiming();
    write();
}

QString GalvoRamp::getPhysicalChannels() const
{
    return physicalChannels;
}

void GalvoRamp::configureTiming()
{
    DAQmxErrChk(
        DAQmxCfgSampClkTiming(
            task,
            nullptr,  // clock source (onboard if NULL)
            rate,  // samples per second per channel
            DAQmx_Val_Rising,
            DAQmx_Val_FiniteSamps,
            static_cast<uInt64>(nSamples)
            )
        );
}

void GalvoRamp::write()
{
    DAQmxErrChk(
        DAQmxWriteAnalogF64(
            task,
            waveform.size(),
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
    QVector<double> temp(nSamples, 0);
    double halfAmplitude = 0.5 * amplitude;
    int i = 0;
    for (; i < nRamp; ++i)
        temp[i] = offset - halfAmplitude + amplitude * i / nRamp;
    int nRamp2 = nSamples - nRamp;
    for (; i < nSamples; ++i)
        temp[i] = offset + halfAmplitude - amplitude * (i - nRamp) / nRamp2;

    if (delay == 0) {
        waveform = temp;
        return;
    }

    i = 0;
    waveform.clear();
    waveform.reserve(nSamples);

    if (delay > 0) {
        waveform << temp.mid(nSamples - delay - 1, delay);
        waveform << temp.mid(0, nSamples - delay);
    } else {
        waveform << temp.mid(delay, nSamples + delay);
        waveform << temp.mid(0, -delay);
    }
}
