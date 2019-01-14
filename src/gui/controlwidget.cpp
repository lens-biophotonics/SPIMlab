#include <QPushButton>
#include <QVBoxLayout>
#include <QSpacerItem>

#include "controlwidget.h"
#include "core/spimhub.h"
#include "core/statemachine.h"

ControlWidget::ControlWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void ControlWidget::setupUi()
{
    QPushButton *initPushButton = new QPushButton("Initialize");
    initPushButton->setObjectName("initPushButton");

    QPushButton *startCapturePushButton = new QPushButton("Start capture");
    startCapturePushButton->setObjectName("startCapturePushButton");

    QPushButton *stopCapturePushButton = new QPushButton("Stop capture");
    stopCapturePushButton->setObjectName("stopCapturePushButton");

    QState *s;

    s = stateMachine().getState(STATE_UNINITIALIZED);
    s->assignProperty(initPushButton, "enabled", true);
    s->assignProperty(startCapturePushButton, "enabled", false);
    s->assignProperty(stopCapturePushButton, "enabled", false);

    s = stateMachine().getState(STATE_READY);
    s->assignProperty(initPushButton, "enabled", false);
    s->assignProperty(startCapturePushButton, "enabled", true);
    s->assignProperty(stopCapturePushButton, "enabled", false);

    s = stateMachine().getState(STATE_CAPTURING);
    s->assignProperty(startCapturePushButton, "enabled", false);
    s->assignProperty(stopCapturePushButton, "enabled", true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(initPushButton);
    layout->addWidget(startCapturePushButton);
    layout->addWidget(stopCapturePushButton);
    layout->addStretch();

    setLayout(layout);

    QMetaObject::connectSlotsByName(this);
}

void ControlWidget::on_initPushButton_clicked()
{
    spimHub().initialize();
}

void ControlWidget::on_startCapturePushButton_clicked()
{
    spimHub().startAcquisition();
}

void ControlWidget::on_stopCapturePushButton_clicked()
{
    spimHub().stop();
}
