#ifndef TASKS_H
#define TASKS_H

#include <QObject>

#include <qtlab/hw/ni/nitask.h>

class GalvoRamp;
class CameraTrigger;

class Tasks : public QObject
{
    Q_OBJECT
public:
    explicit Tasks(QObject *parent = nullptr);
    void init();
    void clearTasks();
    void start();
    void stop();

    GalvoRamp *getGalvoRamp() const;
    CameraTrigger *getCameraTrigger() const;

private:
    CameraTrigger * cameraTrigger;
    GalvoRamp *galvoRamp;

    bool initialized = false;
};

#endif // TASKS_H
