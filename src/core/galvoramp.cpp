#include <iostream>
#include "galvoramp.h"

using namespace std;

#define CHANNEL_NAME "galvoRampAOChan"

GalvoRamp::GalvoRamp() : NIDevice()
{
}

GalvoRamp::~GalvoRamp()
{
    clearTasks();
}

bool GalvoRamp::initializeTasks(QString physicalChannel, QString triggerSource)
{
    clearTasks();
    this->physicalChannel = physicalChannel;
#ifdef WITH_HARDWARE
    DAQmxErrChkRetFalse(DAQmxCreateTask("galvoRampAO", &task));

    DAQmxErrChkRetFalse(
        DAQmxCreateAOVoltageChan(
            task,
            physicalChannel.toLatin1(),
            CHANNEL_NAME,
            -10.0,  // minVal
            10.0,  // maxVal
            DAQmx_Val_Volts,  // units
            NULL  // customScaleName
            )
        );
    DAQmxErrChkRetFalse(DAQmxCfgDigEdgeStartTrig(
                            task,
                            triggerSource.toLatin1(),
                            DAQmx_Val_Rising
                            )
                        );
    DAQmxErrChkRetFalse(DAQmxSetStartTrigRetriggerable(task, true));
#else
    Q_UNUSED(triggerSource)
#endif
    return true;
}

bool GalvoRamp::start()
{
#ifdef WITH_HARDWARE
    DAQmxErrChkRetFalse(
        DAQmxCfgSampClkTiming(
            task,
            NULL,  // clock source (onboard if NULL)
            rate,  // samples per second per channel
            DAQmx_Val_Rising,
            DAQmx_Val_FiniteSamps,
            waveform.size()
            )
        );
    DAQmxErrChkRetFalse(
        DAQmxWriteAnalogF64(
            task,
            waveform.size(),
            false,  // autostart
            10,  // timeout in seconds
            DAQmx_Val_GroupByChannel,
            waveform.data(),
            NULL,
            NULL  // reserved
            )
        );

    DAQmxErrChkRetFalse(DAQmxStartTask(task));
#endif
    return true;
}

bool GalvoRamp::stop()
{
#ifdef WITH_HARDWARE
    DAQmxErrChkRetFalse(DAQmxStopTask(task));
#endif
    return true;
}

bool GalvoRamp::clearTasks()
{
#ifdef WITH_HARDWARE
    if (task)
        DAQmxErrChkRetFalse(DAQmxClearTask(task));

#endif
    return true;
}

void GalvoRamp::createWaveform(uint nSamples, uint nRamp, double offset, double amplitude,
                               int delay,
                               double rate)
{
    this->rate = rate;
    QVector<double> temp(nSamples, 0);
    double halfAmplitude = 0.5 * amplitude;
    uint i = 0;
    for (; i < nRamp; ++i)
        temp[i] = offset - halfAmplitude + amplitude * i / nRamp;
    uint nRamp2 = nSamples - nRamp;
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
        delay = -delay;
        waveform << temp.mid(delay, nSamples - delay);
        waveform << temp.mid(0, delay);
    }
}
