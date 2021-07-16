#include <qtlab/core/logmanager.h>

#include "cameratrigger.h"
#include "spim.h"

#include <QVector>

using namespace NI;

static Logger *logger = getLogger("CameraTrigger");

#define DUTY_CYCLE 0.01
#define DUTY_CYCLE_BLANKING 0.9485
#define CAMERA_ACQUISITION_DELAY 85e-6

CameraTrigger::CameraTrigger(QObject *parent)
    : NITask(parent)
{
}

void CameraTrigger::initializeTask_impl()
{
    if (isInitialized()) {
        clearTask();
    }
    QStringList counters = NI::getCOPhysicalChans().filter("/ctr");
#ifndef WITH_HARDWARE
    for (int i = 0; i < pulseTerms.size(); ++i) {
        counters << "" << "";
    }
#endif
    int counterIdx = 0;

    createTask("Camera trigger");
    int nCams = SPIM_NCAMS;
    for (int i = 0; i < nCams; i++) {
        // camera trigger
        QString chanName = QString("CamTrig%1").arg(i);
        double delay = i * (1 / pulseFreq) / nCams;
        createCOPulseChanFreq(counters.at(counterIdx++), chanName, DAQmx_Val_Hz,
                              IdleState_Low, delay, pulseFreq, 0.1);
        setCOPulseTerm(chanName, pulseTerms.at(i));

        // blanking
        chanName = QString("Blanking%1").arg(i);
        createCOPulseChanFreq(counters.at(counterIdx++), chanName, DAQmx_Val_Hz,
                              IdleState_Low, delay, pulseFreq, 0.9485);
        setCOPulseTerm(chanName, blankingPulseTerms.at(i));
    }


    if (isFreeRun) {
        cfgImplicitTiming(SampMode_ContSamps, 10);
    } else {
        cfgImplicitTiming(SampMode_FiniteSamps, nPulses);
        cfgDigEdgeStartTrig(startTriggerTerm, Edge_Rising);
    }
}

int CameraTrigger::getNPulses() const
{
    return nPulses;
}

void CameraTrigger::setNPulses(int value)
{
    nPulses = value;
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


QString CameraTrigger::getStartTriggerTerm() const
{
    return startTriggerTerm;
}

void CameraTrigger::setStartTriggerTerm(const QString &value)
{
    startTriggerTerm = value;
}

QStringList CameraTrigger::getBlankingPulseTerms() const
{
    return blankingPulseTerms;
}

void CameraTrigger::setBlankingPulseTerms(const QStringList &value)
{
    blankingPulseTerms = value;
}

QStringList CameraTrigger::getPulseTerms() const
{
    return pulseTerms;
}

void CameraTrigger::setPulseTerms(const QStringList &value)
{
    pulseTerms = value;
}

void CameraTrigger::setPulseFreq(double value)
{
    pulseFreq = value;
}

double CameraTrigger::getPulseFreq() const
{
    return pulseFreq;
}
