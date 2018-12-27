#ifndef SPIMEVENT_H
#define SPIMEVENT_H

#include <QEvent>

class SPIMEvent : public QEvent
{
public:
    enum SPIMEventType {
        START_FREE_RUN_REQUESTED,
        STOP_FREE_RUN_REQUESTED,
    };

    static const QEvent::Type TYPE;
    SPIMEvent(SPIMEventType type);

    SPIMEventType getType();

private:
    SPIMEventType type;
};


#endif // SPIMEVENT_H
