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

void CameraTrigger::setPhysicalChannel(const QString &channel)
{
    physicalChannel = channel;
    clear();
}

void CameraTrigger::initializeTask_impl()
{
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

    logger->info(QString("Created Counter Output using %1, terminal: %2").arg(
                     physicalChannel, getTerm()));
}


void CameraTrigger::configureTriggering()
{
    if (isFreeRun) {
        DAQmxErrChk(DAQmxDisableStartTrig(task));
    } else {
        DAQmxErrChk(
            DAQmxCfgDigEdgeStartTrig(
                task, triggerTerm.toLatin1(), DAQmx_Val_Rising));
    }
}

void CameraTrigger::configureTerm()
{
    DAQmxErrChk(DAQmxSetCOPulseTerm(task, CHANNEL_NAME, term.toLatin1()));
}

void CameraTrigger::setFrequency(const double Hz)
{
    freq = Hz;
    if (isInitialized()) {
        DAQmxErrChk(DAQmxSetCOPulseFreq(task, CHANNEL_NAME, freq));
    }
}

double CameraTrigger::getFrequency()
{
    DAQmxErrChk(DAQmxGetCOPulseFreq(task, CHANNEL_NAME, &freq));
    return freq;
}

void CameraTrigger::setTriggerTerm(const QString &term)
{
    triggerTerm = term;
    if (isInitialized()) {
        configureTriggering();
    }
}

void CameraTrigger::setFreeRunEnabled(const bool enable)
{
    isFreeRun = enable;
    if (isInitialized()) {
        configureTriggering();
    }
}

bool CameraTrigger::isFreeRunEnabled() const
{
    return isFreeRun;
}

QString CameraTrigger::getTerm()
{
    char buff[100];
    DAQmxErrChk(DAQmxGetCOPulseTerm(task, CHANNEL_NAME, buff, 100));
    return QString(buff);
}

void CameraTrigger::setTerm(QString term)
{
    this->term = term;
    if (isInitialized()) {
        configureTerm();
    }
}
