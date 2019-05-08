#include <memory>

#include <QHistoryState>

#include "statemachine.h"
#include "spim.h"

State::State(MACHINE_STATE type, QState *parent) : QState(parent), type(type)
{
}

StateMachine::StateMachine() : QStateMachine()
{
    QState *parentState = new QState(this);

    uninitState = newState(STATE_UNINITIALIZED, parentState);
    parentState->setInitialState(uninitState);

    readyState = newState(STATE_READY, parentState);
    capturingState = newState(STATE_CAPTURING, parentState);

    uninitState->addTransition(&spim(), &SPIM::initialized, readyState);
    readyState->addTransition(&spim(), &SPIM::captureStarted, capturingState);
    capturingState->addTransition(&spim(), &SPIM::stopped, readyState);

    QHistoryState *historyState = new QHistoryState(parentState);

    errorState = newState(STATE_ERROR, this);
    errorState->addTransition(historyState);

    parentState->addTransition(&spim(), &SPIM::error, errorState);
    setInitialState(parentState);
    start();
}

QState *StateMachine::getState(const MACHINE_STATE state) const
{
    return map[state];
}

QState *StateMachine::newState(const MACHINE_STATE type, QState *parent)
{
    QState *state = new QState(parent);
    map[type] = state;
    return state;
}

StateMachine &stateMachine()
{
    static auto instance = std::make_unique<StateMachine>();
    return *instance;
}
