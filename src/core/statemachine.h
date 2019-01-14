#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <QStateMachine>
#include <QState>
#include <QAbstractTransition>
#include <QMap>

enum MACHINE_STATE {
    STATE_UNINITIALIZED,
    STATE_READY,
    STATE_CAPTURING,
    STATE_ERROR,
};



class State : public QState
{
public:
    State(MACHINE_STATE type, QState *parent = nullptr);
    const MACHINE_STATE type;
};



class StateMachine : public QStateMachine
{
public:

    StateMachine();

    QState *getState(MACHINE_STATE state);

private:
    QState *uninitState;
    QState *readyState;
    QState *capturingState;
    QState *errorState;

    QState *newState(MACHINE_STATE type, QState *parent = nullptr);

    QMap<MACHINE_STATE, QState *> map;
};


#endif // STATEMACHINE_H
