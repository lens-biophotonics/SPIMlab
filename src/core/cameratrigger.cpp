#include "cameratrigger.h"
#include "logmanager.h"

using namespace NI;

static Logger *logger = getLogger("CameraTrigger");

#define CHANNEL_NAME "cameraTriggerCOPulseChan"

CameraTrigger::CameraTrigger(QObject *parent) : NIAbstractTask(parent)
{
    freq = 50;
    dutyCycle = 0.1;
}

CameraTrigger::~CameraTrigger()
{
    clear();
}

void CameraTrigger::setPhysicalChannel(QString channel)
{
    this->physicalChannel = channel;
}

bool CameraTrigger::initializeTask()
{
#ifdef WITH_HARDWARE
    DAQmxErrChkRetFalse(DAQmxCreateTask("cameraTriggerCOPulse", &task));

    DAQmxErrChkRetFalse(
        DAQmxCreateCOPulseChanFreq(
            task,
            physicalChannel.toLatin1(),
            CHANNEL_NAME,
            DAQmx_Val_Hz,  // units
            DAQmx_Val_Low,  // idleState
            0,  // initialDelay
            freq,
            dutyCycle
            )
        );

    DAQmxErrChkRetFalse(
        DAQmxCfgImplicitTiming(task, DAQmx_Val_ContSamps, 1000));

    DAQmxErrChkRetFalse(DAQmxSetCOPulseFreq(task, CHANNEL_NAME, freq));
    if (isFreeRun) {
        DAQmxErrChkRetFalse(DAQmxDisableStartTrig(task));
    } else {
        DAQmxErrChkRetFalse(
            DAQmxCfgDigEdgeStartTrig(
                task, triggerTerm.toLatin1(), DAQmx_Val_Rising));
    }
#endif

    logger->info(QString("Created Counter Output using %1, terminal: %2").arg(
                     physicalChannel, getTerm()));
    return true;
}

void CameraTrigger::setFrequency(double Hz)
{
    freq = Hz;
}

double CameraTrigger::getFrequency()
{
#ifdef WITH_HARDWARE
    DAQmxErrChkRetFalse(DAQmxGetCOPulseFreq(task, CHANNEL_NAME, &freq));
#endif
    return freq;
}

void CameraTrigger::setTriggerTerm(QString term)
{
    triggerTerm = term;
}

void CameraTrigger::setFreeRunEnabled(bool enable)
{
    isFreeRun = enable;
}

bool CameraTrigger::isFreeRunEnabled()
{
    return isFreeRun;
}

QString CameraTrigger::getTerm()
{
#ifdef WITH_HARDWARE
    char buff[100];
    DAQmxErrChk(DAQmxGetCOPulseTerm(task, CHANNEL_NAME, buff, 100));
    return QString(buff);

#endif
    return QString();
}

bool CameraTrigger::setTerm(QString term)
{
#ifdef WITH_HARDWARE
    DAQmxErrChkRetFalse(DAQmxSetCOPulseTerm(task, CHANNEL_NAME, term.toLatin1()));
#else
    Q_UNUSED(term);
#endif
    return true;
}
