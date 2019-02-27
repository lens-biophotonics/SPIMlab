#ifndef CAMERATRIGGER_H
#define CAMERATRIGGER_H

#include "niabstracttask.h"

class CameraTrigger : public NIAbstractTask
{
public:
    CameraTrigger(QObject *parent = nullptr);

    void setPhysicalChannel(const QString &channel);
    QString getPhysicalChannel() const;

    void setFrequency(const double Hz);
    double getFrequency();

    void setFreeRunEnabled(const bool enable);
    bool isFreeRunEnabled() const;

    QString getTerm();
    void setTerm(QString term);

    void setTriggerTerm(const QString &term);

    NI::float64 getInitialDelay() const;
    void setInitialDelay(const NI::float64 &value);

protected:
    void initializeTask_impl();

private:
    QString physicalChannel;
    QString triggerTerm;
    QString term;
    NI::float64 freq;
    NI::float64 initialDelay = 0;
    bool isFreeRun;

    void configureTriggering();
    void configureTerm();
};

#endif // CAMERATRIGGER_H
