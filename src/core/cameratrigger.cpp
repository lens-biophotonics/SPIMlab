#include "cameratrigger.h"
#include "logmanager.h"

using namespace NI;

static Logger *logger = getLogger("CameraTrigger");

#define CHANNEL_NAME(n) QString("cameraTriggerCOPulseChan_%1").arg(n).toLatin1()
#define DUTY_CYCLE 0.1

CameraTrigger::CameraTrigger(QObject *parent) : NIAbstractTask(parent)
{
    for (int i = 0; i < 2; ++i) {
        freqs.push_back(50);
    }

    setTaskName("cameraTriggerCOPulse");
}

void CameraTrigger::setPhysicalChannels(const QStringList &channels)
{
    physicalChannels = channels;
    clear();
}

void CameraTrigger::initializeTask_impl()
{
    for (int i = 0; i < physicalChannels.count(); ++i) {
        DAQmxErrChk(
            DAQmxCreateCOPulseChanFreq(
                task,
                physicalChannels.at(i).toLatin1(),
                CHANNEL_NAME(i),
                DAQmx_Val_Hz,  // units
                DAQmx_Val_Low,  // idleState
                initialDelays.at(i),
                freqs.at(i),
                DUTY_CYCLE
                )
            );
    }

    DAQmxErrChk(
        DAQmxCfgImplicitTiming(task, DAQmx_Val_ContSamps, 1000));

    configureTriggering();
    configureTerms();

    for (int i = 0; i < physicalChannels.count(); ++i) {
        logger->info(QString("Created Counter Output using %1, terminal: %2")
                     .arg(physicalChannels.at(i), getTerm(i)));
    }
}

QString CameraTrigger::getTriggerTerm() const
{
    return triggerTerm;
}

QStringList CameraTrigger::getPhysicalChannels() const
{
    return physicalChannels;
}

NI::float64 CameraTrigger::getInitialDelay(const int number) const
{
    return initialDelays.at(number);
}

void CameraTrigger::setInitialDelays(const QList<NI::float64> &values)
{
    initialDelays = values;
}

QString CameraTrigger::getPhysicalChannel(const int number) const
{
    return physicalChannels.at(number);
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

void CameraTrigger::configureTerms()
{
    for (int i = 0; i < physicalChannels.count(); ++i) {
        DAQmxErrChk(
            DAQmxSetCOPulseTerm(task, CHANNEL_NAME(i), terms.at(i).toLatin1()));
    }
}

void CameraTrigger::setFrequencies(const QList<double> Hz)
{
    freqs = Hz;
    if (isInitialized()) {
        for (int i = 0; i < physicalChannels.count(); ++i) {
            DAQmxErrChk(DAQmxSetCOPulseFreq(task, CHANNEL_NAME(i), freqs.at(i)));
        }
    }
}

void CameraTrigger::setFrequency(const double Hz)
{
    QList<double> list;
    list.reserve(physicalChannels.count());
    for (int i = 0; i < physicalChannels.count(); ++i) {
        list.insert(i, Hz);
    }
    setFrequencies(list);
}

double CameraTrigger::getFrequency(const int number)
{
    double temp;
    DAQmxErrChk(DAQmxGetCOPulseFreq(task, CHANNEL_NAME(number), &temp));
    freqs.replace(number, temp);
    return freqs.at(number);
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

QString CameraTrigger::getTerm(const int number)
{
    if (!isInitialized()) {
        return this->terms.at(number);
    }
    char buff[100];
    DAQmxErrChk(DAQmxGetCOPulseTerm(task, CHANNEL_NAME(number), buff, 100));
    return QString(buff);
}

QStringList CameraTrigger::getTerms()
{
    return terms;
}

void CameraTrigger::setTerms(QStringList terms)
{
    this->terms = terms;
    if (isInitialized()) {
        configureTerms();
    }
}
