#include "cameratrigger.h"
#include "logmanager.h"

#include <QVector>

using namespace NI;

static Logger *logger = getLogger("CameraTrigger");

#define DUTY_CYCLE 0.01
#define DUTY_CYCLE_BLANKING 0.9485
#define CAMERA_ACQUISITION_DELAY 85e-6

CameraTrigger::CameraTrigger(QObject *parent) : NIAbstractTask(parent)
{
    setTaskName("cameraTriggerDO");
}

void CameraTrigger::initializeTask_impl()
{
    DAQmxErrChk(
        DAQmxCreateDOChan(
            task,
            physicalChannels.join(",").toLatin1(),
            nullptr,
            DAQmx_Val_ChanPerLine
            )
        );

    configureTriggering();

    configureSampleClockTiming("", DAQmx_Val_Rising, DAQmx_Val_ContSamps);

    QVector<uInt8> waveformTrigger;
    int ns = static_cast<int>(nSamples);
    int nHigh = static_cast<int>(ns * DUTY_CYCLE);
    int nLow = ns - nHigh;
    waveformTrigger << QVector<uInt8>(nHigh, 1) << QVector<uInt8>(nLow, 0);

    QVector<uInt8> waveformBlanking;
    double sr = 1 / getSampleRate();
    nHigh = static_cast<int>(ns * DUTY_CYCLE_BLANKING);
    int nLowStart = static_cast<int>(CAMERA_ACQUISITION_DELAY / sr);
    int nLowEnd = ns - nHigh - nLowStart;
    waveformBlanking << QVector<uInt8>(nLowStart, 0)
                     << QVector<uInt8>(nHigh, 1)
                     << QVector<uInt8>(nLowEnd, 0);

    QVector<uInt8> waveform;
    int nChannels = physicalChannels.size();

    int nCameras = nChannels / 2; // divide by 2 to account for trigger + blanking
    int delay = ns / nCameras;

    for (int i = 0; i < nChannels; ++i) {
        int splitIdx = delay * i; // hardcoded delay between triggers for different cameras
        waveform << waveformTrigger.mid(ns - splitIdx);
        waveform << waveformTrigger.mid(0, ns - splitIdx);

        waveform << waveformBlanking.mid(ns - splitIdx);
        waveform << waveformBlanking.mid(0, ns - splitIdx);
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
    if (task) {
        configureTriggering();
    }
}

bool CameraTrigger::isFreeRunEnabled() const
{
    return isFreeRun;
}
