#include "spimevent.h"

const QEvent::Type SPIMEvent::TYPE =
    (QEvent::Type)(QEvent::registerEventType());

SPIMEvent::SPIMEvent(SPIMEventType type) : QEvent(TYPE)
{
    this->type = type;
}

SPIMEvent::SPIMEventType SPIMEvent::getType()
{
    return type;
}
