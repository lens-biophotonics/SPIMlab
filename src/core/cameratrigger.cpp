#include "cameratrigger.h"
#include "logmanager.h"

#include <QVector>

using namespace NI;

static Logger *logger = getLogger("CameraTrigger");

#define DUTY_CYCLE 0.01

CameraTrigger::CameraTrigger(QObject *parent) : NIAbstractTask(parent)
{
    setTaskName("cameraTriggerDO");
}

void CameraTrigger::initializeTask_impl()
{
    DAQmxErrChk(
        DAQmxCreateDOChan(
            task,
            physicalChannels.join(":").toLatin1(),
            nullptr,
            DAQmx_Val_ChanPerLine
            )
        );

    configureTriggering();

    configureSampleClockTiming("", DAQmx_Val_Rising, DAQmx_Val_ContSamps);

    QVector<uInt8> temp;
    int ns = static_cast<int>(nSamples);
    int nHigh = static_cast<int>(ns * DUTY_CYCLE);
    int nLow = ns - nHigh;
    temp << QVector<uInt8>(nHigh, 1);
    temp << QVector<uInt8>(nLow, 0);

    QVector<uInt8> waveform;
    int nChannels = physicalChannels.size();
    int delay = ns / nChannels;

    waveform << temp;

    for (int i = 1; i < nChannels; ++i) {
        int splitIdx = delay * i;
        waveform << temp.mid(ns - splitIdx);
        waveform << temp.mid(0, ns - splitIdx);
    }

    DAQmxErrChk(
        DAQmxWriteDigitalLines(
            task,
            static_cast<int32>(nSamples),
            false,                // autostart
            0,                    // timeout
            DAQmx_Val_GroupByChannel,
            waveform.data(),
            nullptr,              // sampsPerChanWritten
            nullptr               // reserved
            )
        );
}

void CameraTrigger::configureTriggering()
{
    if (isFreeRun) {
        DAQmxErrChk(DAQmxDisableStartTrig(task));
    } else {
        NIAbstractTask::configureTriggering();
    }
}

void CameraTrigger::setFreeRunEnabled(const bool enable)
{
    isFreeRun = enable;
}

bool CameraTrigger::isFreeRunEnabled() const
{
    return isFreeRun;
}
