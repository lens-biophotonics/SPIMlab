#ifndef CAMERATRIGGER_H
#define CAMERATRIGGER_H

#include <qtlab/hw/ni/niabstracttask.h>

class CameraTrigger : public NIAbstractTask
{
public:
    CameraTrigger(QObject *parent = nullptr);

    void setFreeRunEnabled(const bool enable);
    bool isFreeRunEnabled() const;

protected:
    void initializeTask_impl();
    virtual void configureTriggering();

private:
    bool isFreeRun;
};

#endif // CAMERATRIGGER_H
