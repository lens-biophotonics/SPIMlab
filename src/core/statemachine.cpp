#include "statemachine.h"
#include "spimhub.h"

State::State(MACHINE_STATE type, QState *parent) : QState(parent), type(type)
{
}

StateMachine::StateMachine() : QStateMachine()
{
    SPIMHub *hub = SPIMHub::getInstance();

    uninitState = newState(STATE_UNINITIALIZED);
    setInitialState(uninitState);

    errorState = newState(STATE_ERROR);
    setErrorState(errorState);

    readyState = newState(STATE_READY);
    capturingState = newState(STATE_CAPTURING);

    uninitState->addTransition(hub, &SPIMHub::initialized, readyState);
    readyState->addTransition(hub, &SPIMHub::captureStarted, capturingState);
    capturingState->addTransition(hub, &SPIMHub::stopped, readyState);

    QMetaObject::connectSlotsByName(this);

    start();
}

QState *StateMachine::getState(MACHINE_STATE state)
{
    return map[state];
}

QState *StateMachine::newState(MACHINE_STATE type, QState *parent)
{
    QState *state = new QState(parent);
    map[type] = state;
    addState(state);
    return state;
}
