#ifndef CAMERATRIGGER_H
#define CAMERATRIGGER_H

#include "niabstracttask.h"

class CameraTrigger : public NIAbstractTask
{
public:
    CameraTrigger(QObject *parent = nullptr);

    void setPhysicalChannels(const QStringList &channels);
    QString getPhysicalChannel(const int number) const;

    void setFrequencies(const QList<double> Hz);
    void setFrequency(const double Hz);
    double getFrequency(const int number);

    void setFreeRunEnabled(const bool enable);
    bool isFreeRunEnabled() const;

    QString getTerm(const int number);
    void setTerms(QStringList terms);

    void setTriggerTerm(const QString &term);

    NI::float64 getInitialDelay(const int number) const;
    void setInitialDelays(const QList<NI::float64> &values);

protected:
    void initializeTask_impl();

private:
    QStringList physicalChannels;
    QString triggerTerm;
    QStringList terms;
    QList<NI::float64> freqs;
    QList<NI::float64> initialDelays;
    bool isFreeRun;

    void configureTriggering();
    void configureTerms();
};

#endif // CAMERATRIGGER_H
