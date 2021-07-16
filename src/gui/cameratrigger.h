#ifndef CAMERATRIGGER_H
#define CAMERATRIGGER_H

#include <qtlab/hw/ni/nitask.h>

class CameraTrigger : public NITask
{
public:
    CameraTrigger(QObject *parent = nullptr);

    void setFreeRunEnabled(const bool enable);
    bool isFreeRunEnabled() const;

protected:
    virtual void initializeTask_impl() override;

private:
    bool isFreeRun;
};

#endif // CAMERATRIGGER_H
