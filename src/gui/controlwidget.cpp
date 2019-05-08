#include <QPushButton>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QLabel>
#include <QGroupBox>

#include "controlwidget.h"
#include "core/spim.h"

ControlWidget::ControlWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void ControlWidget::setupUi()
{
    QPushButton *initPushButton = new QPushButton("Initialize");
    connect(initPushButton, &QPushButton::clicked, &spim(), [ = ](){
        initPushButton->setEnabled(false);
        spim().initialize();
    });

    QPushButton *startFreeRunPushButton = new QPushButton("Start free run");
    connect(startFreeRunPushButton, &QPushButton::clicked,
            &spim(), &SPIM::startFreeRun);

    QPushButton *startAcqPushButton = new QPushButton("Start acquisition");
    connect(startAcqPushButton, &QPushButton::clicked,
            &spim(), &SPIM::startAcquisition);

    QPushButton *stopCapturePushButton = new QPushButton("Stop capture");
    connect(stopCapturePushButton, &QPushButton::clicked,
            &spim(), &SPIM::stop);

    QLabel *statusLabel = new QLabel();
    statusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QState *s;

    s = spim().getState(SPIM::STATE_UNINITIALIZED);
    s->assignProperty(initPushButton, "enabled", true);
    s->assignProperty(startFreeRunPushButton, "enabled", false);
    s->assignProperty(startAcqPushButton, "enabled", false);
    s->assignProperty(stopCapturePushButton, "enabled", false);
    s->assignProperty(statusLabel, "text", "Uninitialized");

    s = spim().getState(SPIM::STATE_READY);
    s->assignProperty(initPushButton, "enabled", false);
    s->assignProperty(startFreeRunPushButton, "enabled", true);
    s->assignProperty(startAcqPushButton, "enabled", true);
    s->assignProperty(stopCapturePushButton, "enabled", false);
    s->assignProperty(statusLabel, "text", "Ready");

    s = spim().getState(SPIM::STATE_CAPTURING);
    s->assignProperty(startFreeRunPushButton, "enabled", false);
    s->assignProperty(startAcqPushButton, "enabled", false);
    s->assignProperty(stopCapturePushButton, "enabled", true);
    s->assignProperty(statusLabel, "text", "Capturing");

    spim().getState(SPIM::STATE_PRECAPTURE)->assignProperty(
        statusLabel, "text", "Precapture");
    spim().getState(SPIM::STATE_CAPTURE)->assignProperty(
        statusLabel, "text", "Capture");
    spim().getState(SPIM::STATE_FREERUN)->assignProperty(
        statusLabel, "text", "Free run");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(initPushButton);
    layout->addWidget(startFreeRunPushButton);
    layout->addWidget(startAcqPushButton);
    layout->addWidget(stopCapturePushButton);
    layout->addStretch();
    layout->addWidget(statusLabel);

    QGroupBox *gb = new QGroupBox("Controls");
    gb->setLayout(layout);

    layout = new QVBoxLayout();
    layout->addWidget(gb);
    setLayout(layout);

    QMetaObject::connectSlotsByName(this);
}
