#include <qtlab/core/logmanager.h>

#include "cameratrigger.h"

#include <QVector>

using namespace NI;

static Logger *logger = getLogger("CameraTrigger");

#define DUTY_CYCLE 0.01
#define DUTY_CYCLE_BLANKING 0.9485
#define CAMERA_ACQUISITION_DELAY 85e-6

CameraTrigger::CameraTrigger(QObject *parent)
    : NITask(parent)
{
    setTaskName("cameraTriggerDO");
}

void CameraTrigger::initializeTask_impl()
{
    NITask::initializeTask_impl();

    // waveform
    QVector<uInt8> waveformTrigger;
    int ns = static_cast<int>(sampsPerChan);
    int nHigh = static_cast<int>(ns * DUTY_CYCLE);
    int nLow = ns - nHigh;
    waveformTrigger << QVector<uInt8>(nHigh, 1) << QVector<uInt8>(nLow, 0);

    QVector<uInt8> waveformBlanking;
    nHigh = static_cast<int>(ns * DUTY_CYCLE_BLANKING);
    int nLowStart = static_cast<int>(CAMERA_ACQUISITION_DELAY * getSampleRate());
    int nLowEnd = ns - nHigh - nLowStart;
    waveformBlanking << QVector<uInt8>(nLowStart, 0)
                     << QVector<uInt8>(nHigh, 1)
                     << QVector<uInt8>(nLowEnd, 0);

    QVector<uInt8> waveform;
    int nChannels = physicalChannels.size();

    int nCameras = nChannels / 2; // divide by 2 to account for trigger + blanking
    int delay = ns / nCameras;

    for (int i = 0; i < nChannels; ++i) {
        int splitIdx = delay * i; // delay between triggers for different cameras
        waveform << waveformTrigger.mid(ns - splitIdx);
        waveform << waveformTrigger.mid(0, ns - splitIdx);

        waveform << waveformBlanking.mid(ns - splitIdx);
        waveform << waveformBlanking.mid(0, ns - splitIdx);
    }

    writeDigitalLines(
        static_cast<int32>(sampsPerChan),
        false,          //autostart
        0,              // timeout
        DataLayout_GroupByChannel,
        waveform.data(),
        nullptr         // sampsPerChanWritten
        );
}

void CameraTrigger::configureChannels_impl()
{
    createDOChan(physicalChannels.join(",").toLatin1(),
                 nullptr, LineGrp_ChanPerLine);
}

void CameraTrigger::configureTiming_impl()
{
    cfgSampClkTiming(
        "", sampleRate, Edge_Rising, SampMode_ContSamps, sampsPerChan);
}

void CameraTrigger::configureTriggering_impl()
{
    if (isFreeRun) {
        disableStartTrig();
    } else {
        cfgDigEdgeStartTrig(triggerTerm.toLatin1(), Edge_Rising);
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
