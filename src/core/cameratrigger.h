#ifndef CAMERATRIGGER_H
#define CAMERATRIGGER_H

#include "niabstracttask.h"

class CameraTrigger : public NIAbstractTask
{
public:
    CameraTrigger();
    ~CameraTrigger();

    void setPhysicalChannel(QString channel);
    bool initializeTask();

    void setFrequency(double Hz);
    double getFrequency();

    void setFreeRunEnabled(bool enable);
    bool isFreeRunEnabled();

    QString getTerm();
    bool setTerm(QString term);

    void setTriggerTerm(QString term);

private:
    QString physicalChannel;
    QString triggerTerm;
    NI::float64 freq;
    NI::float64 dutyCycle;
    bool isFreeRun;
};

#endif // CAMERATRIGGER_H
