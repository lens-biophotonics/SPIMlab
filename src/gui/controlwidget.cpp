#include <QPushButton>
#include <QVBoxLayout>
#include <QSpacerItem>

#include "controlwidget.h"
#include "core/spimhub.h"

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

    StateMachine *sm = SPIMHub::getInstance()->stateMachine();
    QState *s;

    s = sm->getState(STATE_UNINITIALIZED);
    s->assignProperty(initPushButton, "enabled", true);
    s->assignProperty(startCapturePushButton, "enabled", false);
    s->assignProperty(stopCapturePushButton, "enabled", false);

    s = sm->getState(STATE_READY);
    s->assignProperty(initPushButton, "enabled", false);
    s->assignProperty(startCapturePushButton, "enabled", true);
    s->assignProperty(stopCapturePushButton, "enabled", false);

    s = sm->getState(STATE_CAPTURING);
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
    SPIMHub::getInstance()->initialize();
}

void ControlWidget::on_startCapturePushButton_clicked()
{
    SPIMHub::getInstance()->startAcquisition();
}

void ControlWidget::on_stopCapturePushButton_clicked()
{
    SPIMHub::getInstance()->stop();
}
