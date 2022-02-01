#include "tasks.h"

#include "cameratrigger.h"
#include "galvoramp.h"
#include "spim.h"

Tasks::Tasks(QObject *parent)
    : QObject(parent)
{
    cameraTrigger = new CameraTrigger;
    galvoRamp = new GalvoRamp;
}

void Tasks::init()
{
    clearTasks();
    cameraTrigger->initializeTask();
    galvoRamp->setTriggerTerm(cameraTrigger->getPulseTerms().at(0));
    galvoRamp->initializeTask();
}

void Tasks::start()
{
    if (!initialized) {
        init();
    }
    galvoRamp->startTask();
    cameraTrigger->startTask();
}

void Tasks::stop()
{
    QList<NITask *> list;
    list << cameraTrigger;
    list << galvoRamp;
    for (NITask *t : list) {
        if (t->isInitialized()) {
            t->stopTask();
        }
    }
}

void Tasks::clearTasks()
{
    initialized = false;
    QList<NITask *> list;
    list << cameraTrigger;
    list << galvoRamp;
    for (NITask *t : list) {
        if (t->isInitialized()) {
            t->clearTask();
        }
    }
}

GalvoRamp *Tasks::getGalvoRamp() const
{
    return galvoRamp;
}

CameraTrigger *Tasks::getCameraTrigger() const
{
    return cameraTrigger;
}
