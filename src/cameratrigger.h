#ifndef CAMERATRIGGER_H
#define CAMERATRIGGER_H

#include "nidevice.h"

class CameraTrigger : public NIDevice
{
public:
    CameraTrigger();
    ~CameraTrigger();

    bool initializeTasks(QString physicalChannel);

    void setFrequency(double Hz);
    double getFrequency();

    void setFreeRunEnabled(bool enable);
    bool isFreeRunEnabled();

    QString getTerm();
    bool setTerm(QString term);

    void setTriggerTerm(QString term);

    bool start();
    bool stop();

private:
    TaskHandle task;
    QString physicalChannel, triggerTerm;
    float64 freq, dutyCycle;
    bool isFreeRun;

    bool clearTasks();
};

#endif // CAMERATRIGGER_H
