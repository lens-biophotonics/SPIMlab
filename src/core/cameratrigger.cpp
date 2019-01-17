#include "cameratrigger.h"
#include "logmanager.h"

using namespace NI;

static Logger *logger = getLogger("CameraTrigger");

#define CHANNEL_NAME "cameraTriggerCOPulseChan"
#define DUTY_CYCLE 0.1

CameraTrigger::CameraTrigger(QObject *parent) : NIAbstractTask(parent)
{
    freq = 50;
}

CameraTrigger::~CameraTrigger()
{
    clear();
}

void CameraTrigger::setPhysicalChannel(QString channel)
{
    physicalChannel = channel;
    clear();
}

void CameraTrigger::initializeTask_impl()
{
#ifdef WITH_HARDWARE
    DAQmxErrChk(DAQmxCreateTask("cameraTriggerCOPulse", &task));

    DAQmxErrChk(
        DAQmxCreateCOPulseChanFreq(
            task,
            physicalChannel.toLatin1(),
            CHANNEL_NAME,
            DAQmx_Val_Hz,  // units
            DAQmx_Val_Low,  // idleState
            0,  // initialDelay
            freq,
            DUTY_CYCLE
            )
        );

    DAQmxErrChk(
        DAQmxCfgImplicitTiming(task, DAQmx_Val_ContSamps, 1000));

    configureTriggering();
    configureTerm();
#endif

    logger->info(QString("Created Counter Output using %1, terminal: %2").arg(
                     physicalChannel, getTerm()));
}


void CameraTrigger::configureTriggering()
{
#ifdef WITH_HARDWARE
    if (isFreeRun) {
        DAQmxErrChk(DAQmxDisableStartTrig(task));
    } else {
        DAQmxErrChk(
            DAQmxCfgDigEdgeStartTrig(
                task, triggerTerm.toLatin1(), DAQmx_Val_Rising));
    }
#endif
}

void CameraTrigger::configureTerm()
{
    DAQmxErrChk(DAQmxSetCOPulseTerm(task, CHANNEL_NAME, term.toLatin1()));
}

void CameraTrigger::setFrequency(double Hz)
{
    freq = Hz;
#ifdef WITH_HARDWARE
    if (isInitialized()) {
        DAQmxErrChk(DAQmxSetCOPulseFreq(task, CHANNEL_NAME, freq));
    }
#endif
}

double CameraTrigger::getFrequency()
{
#ifdef WITH_HARDWARE
    DAQmxErrChk(DAQmxGetCOPulseFreq(task, CHANNEL_NAME, &freq));
#endif
    return freq;
}

void CameraTrigger::setTriggerTerm(QString term)
{
    triggerTerm = term;
    if (isInitialized()) {
        configureTriggering();
    }
}

void CameraTrigger::setFreeRunEnabled(bool enable)
{
    isFreeRun = enable;
    if (isInitialized()) {
        configureTriggering();
    }
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

void CameraTrigger::setTerm(QString term)
{
    this->term = term;
    if (isInitialized()) {
        configureTerm();
    }
}
