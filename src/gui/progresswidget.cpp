#include <cmath>

#include <QGroupBox>
#include <QGridLayout>
#include <QProgressBar>
#include <QLabel>
#include <QDateTime>
#include <QTimer>

#include "spim.h"

#include "progresswidget.h"

ProgressWidget::ProgressWidget(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void ProgressWidget::setupUI()
{
    QGridLayout *grid = new QGridLayout();

    int row = 0;
    int col = 0;

    QProgressBar *stackProgressBar = new QProgressBar();
    QProgressBar *progressBar = new QProgressBar();

    stackProgressBar->setSizePolicy(
        QSizePolicy::Minimum, progressBar->sizePolicy().verticalPolicy());
    progressBar->setSizePolicy(
        QSizePolicy::Expanding, progressBar->sizePolicy().verticalPolicy());

    stackProgressBar->setFormat("%p% (stack)");
    progressBar->setFormat("%p% (%v/%m)");

    grid->addWidget(stackProgressBar, row, col++, 1, 1);
    grid->addWidget(progressBar, row++, col, 1, -1);

    col = 0;
    grid->addWidget(new QLabel("Time / ETA:"), row, col++);

    QLabel *timeLabel = new QLabel();
    grid->addWidget(timeLabel, row, col++);
    QLabel *etaLabel = new QLabel();
    grid->addWidget(etaLabel, row, col++);

    QGroupBox *gb = new QGroupBox("Progress");
    gb->setLayout(grid);

    QBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(gb);

    setLayout(layout);

    QDateTime *startDateTime = new QDateTime();

    QState *s;

    s = spim().getState(SPIM::STATE_ACQUISITION);
    connect(s, &QState::entered, this, [ = ](){
        *startDateTime = QDateTime::currentDateTime();
        timeLabel->clear();

        qint64 remainingSeconds = static_cast<qint64>(
            spim().getTotalSteps()
            * spim().getNSteps(spim().getStackStage())
            / spim().getTriggerRate());

        etaLabel->setText(startDateTime->addSecs(remainingSeconds).toString());
        progressBar->setRange(0, spim().getTotalSteps());
        progressBar->setValue(0);

        // times 1000 to reduce round error in progress (when multiplying by
        // the trigger rate)
        stackProgressBar->setRange(
            0, spim().getNSteps(spim().getStackStage()) * 1000);
    });

    QTimer *stackProgressTimer = new QTimer();

    connect(stackProgressTimer, &QTimer::timeout, this, [ = ](){
        stackProgressBar->setValue(
            stackProgressBar->value() + spim().getTriggerRate() * 1000);
    });

    s = spim().getState(SPIM::STATE_CAPTURE);
    connect(s, &QState::entered, this, [ = ](){
        stackProgressBar->setValue(0);
        stackProgressTimer->start(1000);
    });

    connect(s, &QState::exited, this, [ = ](){
        stackProgressTimer->stop();
        stackProgressBar->reset();
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
    });
}
