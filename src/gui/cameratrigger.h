#ifndef CAMERATRIGGER_H
#define CAMERATRIGGER_H

#include <qtlab/hw/ni/nitask.h>

class CameraTrigger : public NITask
{
public:
    CameraTrigger(QObject *parent = nullptr);

    void setFreeRunEnabled(const bool enable);
    bool isFreeRunEnabled() const;

    double getPulseFreq() const;
    void setPulseFreq(double value);

    QStringList getPulseTerms() const;
    void setPulseTerms(const QStringList &value);

    QStringList getBlankingPulseTerms() const;
    void setBlankingPulseTerms(const QStringList &value);

    QString getStartTriggerTerm() const;
    void setStartTriggerTerm(const QString &value);

    int getNPulses() const;
    void setNPulses(int value);

protected:
    virtual void initializeTask_impl() override;

private:
    bool isFreeRun;
    double pulseFreq;
    int nPulses = 0;
    QString startTriggerTerm;
    QStringList pulseTerms;
    QStringList blankingPulseTerms;
};

#endif // CAMERATRIGGER_H
