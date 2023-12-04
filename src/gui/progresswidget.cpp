#include "progresswidget.h"

#include "savestackworker.h"
#include "spim.h"

#include <cmath>

#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>

ProgressWidget::ProgressWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void ProgressWidget::setupUI()
{
    QList<QProgressBar *> stackPbList;

    QProgressBar *progressBar = new QProgressBar();

    QVBoxLayout *stackPbLayout = new QVBoxLayout();

    for (int i = 0; i < SPIM_NCAMS; ++i) {
        QProgressBar *stackPb = new QProgressBar();
        stackPb->setSizePolicy(QSizePolicy::Minimum, progressBar->sizePolicy().verticalPolicy());
        stackPb->setFormat(QString("%p% (cam %1)").arg(i));
        stackPbList << stackPb;
        stackPbLayout->addWidget(stackPb);
    }

    progressBar->setSizePolicy(QSizePolicy::Expanding, progressBar->sizePolicy().verticalPolicy());

    progressBar->setFormat("%p% (%v/%m)");

    QHBoxLayout *labelHlayout = new QHBoxLayout;
    QLabel *timeLabel = new QLabel();
    labelHlayout->addWidget(timeLabel);
    QLabel *etaLabel = new QLabel();
    labelHlayout->addWidget(etaLabel);

    QVBoxLayout *progressLayout = new QVBoxLayout;
    progressLayout->addWidget(progressBar);
    progressLayout->addLayout(labelHlayout);

    QHBoxLayout *hLayout = new QHBoxLayout;

    hLayout->addLayout(stackPbLayout);
#ifdef MASTER_SPIM
    hLayout->addLayout(progressLayout);
#endif

    QGroupBox *gb = new QGroupBox("Progress");
    gb->setLayout(hLayout);

    QBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(gb);

    setLayout(layout);

    QDateTime *startDateTime = new QDateTime();
    QTimer *timer = new QTimer();

    QState *s;

    s = spim().getState(SPIM::STATE_ACQUISITION);
    connect(s, &QState::entered, this, [=]() {
        *startDateTime = QDateTime::currentDateTime();
        timeLabel->clear();

        qint64 remainingSeconds = static_cast<qint64>(spim().getTotalSteps()
                                                      * spim().getNSteps(spim().getStackStage())
                                                      / spim().getTriggerRate());

        etaLabel->setText(startDateTime->addSecs(remainingSeconds).toString());
        progressBar->setRange(0, spim().getTotalSteps());
        progressBar->setValue(0);
    });

    connect(timer, &QTimer::timeout, this, [=]() {
        for (int i = 0; i < SPIM_NCAMS; ++i) {
            if (spim().isCameraEnabled(i) == 1) {
                stackPbList.at(i)->setRange(0, spim().getSSWorker(i)->getFrameCount());
                stackPbList.at(i)->setValue(spim().getSSWorker(i)->getReadFrames());
            }
        }
    });

    s = spim().getState(SPIM::STATE_CAPTURE);
    connect(s, &QState::entered, this, [=]() {
        timer->start(1000);
        for (int i = 0; i < SPIM_NCAMS; ++i) {
            stackPbList.at(i)->reset();
        }
    });
    connect(s, &QState::exited, timer, &QTimer::stop);

    connect(s, &QState::exited, this, [=]() {
        qint64 seconds = startDateTime->secsTo(QDateTime::currentDateTime());
        int currentStep = spim().getCurrentStep();

        qint64 remainingSeconds = static_cast<qint64>(
            round(seconds / (currentStep + 1) * progressBar->maximum()));

        int h = static_cast<int>(seconds / 3600);
        int m = (seconds % 3600) / 60;
        int s = (seconds % 3600) % 60;

        timeLabel->setText(QString("%1:%2:%3")
                               .arg(h, 2, 10, QChar('0'))
                               .arg(m, 2, 10, QChar('0'))
                               .arg(s, 2, 10, QChar('0')));
        etaLabel->setText(startDateTime->addSecs(remainingSeconds).toString());
        progressBar->setValue(currentStep);
        for (int i = 0; i < SPIM_NCAMS; ++i) {
            if (spim().isCameraEnabled(i) == 1) {
                stackPbList.at(i)->setValue(spim().getSSWorker(i)->getReadFrames());
            }
        }
    });
}
