#include <QPushButton>
#include <QVBoxLayout>
#include <QSpacerItem>

#include "controlwidget.h"
#include "core/spim.h"
#include "core/statemachine.h"

ControlWidget::ControlWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void ControlWidget::setupUi()
{
    QPushButton *initPushButton = new QPushButton("Initialize");
    connect(initPushButton, &QPushButton::clicked, &spim(), &SPIM::initialize);

    QPushButton *startFreeRunPushButton = new QPushButton("Start free run");
    connect(startFreeRunPushButton, &QPushButton::clicked,
            &spim(), &SPIM::startFreeRun);

    QPushButton *startAcqPushButton = new QPushButton("Start acquisition");
    connect(startAcqPushButton, &QPushButton::clicked,
            &spim(), &SPIM::startAcquisition);

    QPushButton *stopCapturePushButton = new QPushButton("Stop capture");
    connect(stopCapturePushButton, &QPushButton::clicked,
            &spim(), &SPIM::stop);

    QState *s;

    s = stateMachine().getState(STATE_UNINITIALIZED);
    s->assignProperty(initPushButton, "enabled", true);
    s->assignProperty(startFreeRunPushButton, "enabled", false);
    s->assignProperty(startAcqPushButton, "enabled", false);
    s->assignProperty(stopCapturePushButton, "enabled", false);

    s = stateMachine().getState(STATE_READY);
    s->assignProperty(initPushButton, "enabled", false);
    s->assignProperty(startFreeRunPushButton, "enabled", true);
    s->assignProperty(startAcqPushButton, "enabled", true);
    s->assignProperty(stopCapturePushButton, "enabled", false);

    s = stateMachine().getState(STATE_CAPTURING);
    s->assignProperty(startFreeRunPushButton, "enabled", false);
    s->assignProperty(startAcqPushButton, "enabled", false);
    s->assignProperty(stopCapturePushButton, "enabled", true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(initPushButton);
    layout->addWidget(startFreeRunPushButton);
    layout->addWidget(startAcqPushButton);
    layout->addWidget(stopCapturePushButton);
    layout->addStretch();

    setLayout(layout);

    QMetaObject::connectSlotsByName(this);
}
