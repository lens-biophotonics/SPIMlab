#include "statemachine.h"
#include "spimhub.h"

State::State(MACHINE_STATE type, QState *parent) : QState(parent), type(type)
{
}

StateMachine::StateMachine() : QStateMachine()
{
    uninitState = newState(STATE_UNINITIALIZED);
    setInitialState(uninitState);

    errorState = newState(STATE_ERROR);
    setErrorState(errorState);

    readyState = newState(STATE_READY);
    capturingState = newState(STATE_CAPTURING);

    uninitState->addTransition(&spimHub(), &SPIMHub::initialized, readyState);
    readyState->addTransition(&spimHub(), &SPIMHub::captureStarted, capturingState);
    capturingState->addTransition(&spimHub(), &SPIMHub::stopped, readyState);

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

StateMachine &stateMachine()
{
    static auto instance = std::make_unique<StateMachine>();
    return *instance;
}
