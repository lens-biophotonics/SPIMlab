#ifndef CAMERATRIGGER_H
#define CAMERATRIGGER_H

#include <qtlab/hw/ni/nitask.h>

#include <QThread>

class TaskWaiter : public QThread
{
    Q_OBJECT
public:
    explicit TaskWaiter(NITask *task, QObject *parent = nullptr)
        : task(task)
        , QThread(parent)
    {
        setTerminationEnabled(true);
    }

    void run() override
    {
        while (!isInterruptionRequested()) {
            try {
                task->waitUntilTaskDone(1);
                emit done();
                break;
            } catch (NITaskError e) {
                if (e.errCode() == DAQmxErrorInvalidTask) {
                    break;
                }
                if (e.errCode() != DAQmxErrorWaitUntilDoneDoesNotIndicateDone) {
                    throw e;
                    break;
                }
            }
        }
    }

signals:
    void error(QString msg);
    void done();

private:
    NITask *task;
};

class CameraTrigger : public NITask
{
    Q_OBJECT

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

signals:
    void done();

protected:
    virtual void initializeTask_impl() override;

private:
    bool isFreeRun;
    double pulseFreq;
    int nPulses = 0;
    QString startTriggerTerm;
    QStringList pulseTerms;
    QStringList blankingPulseTerms;

    TaskWaiter *waiter;
};

#endif // CAMERATRIGGER_H
