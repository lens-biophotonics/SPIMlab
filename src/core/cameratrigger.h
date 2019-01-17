#ifndef CAMERATRIGGER_H
#define CAMERATRIGGER_H

#include "niabstracttask.h"

class CameraTrigger : public NIAbstractTask
{
public:
    CameraTrigger(QObject *parent = nullptr);
    ~CameraTrigger();

    void setPhysicalChannel(QString channel);
    bool initializeTask_impl();

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
    bool isFreeRun;

    bool configureTriggering();
};

#endif // CAMERATRIGGER_H
