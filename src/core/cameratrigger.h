#ifndef CAMERATRIGGER_H
#define CAMERATRIGGER_H

#include "niabstracttask.h"

class CameraTrigger : public NIAbstractTask
{
public:
    CameraTrigger(QObject *parent = nullptr);
    ~CameraTrigger();

    void setPhysicalChannel(QString channel);

    void setFrequency(double Hz);
    double getFrequency();

    void setFreeRunEnabled(bool enable);
    bool isFreeRunEnabled();

    QString getTerm();
    void setTerm(QString term);

    void setTriggerTerm(QString term);

protected:
    void initializeTask_impl();

private:
    QString physicalChannel;
    QString triggerTerm;
    QString term;
    NI::float64 freq;
    bool isFreeRun;

    void configureTriggering();
    void configureTerm();
};

#endif // CAMERATRIGGER_H
