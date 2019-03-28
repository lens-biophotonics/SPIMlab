#include "niabstracttask.h"
#include "logger.h"

using namespace NI;

static Logger *logger = getLogger("NI");

#define ERRBUF_SIZE 2048

NIAbstractTask::NIAbstractTask(QObject *parent) : QObject(parent)
{
    errBuff = new char[ERRBUF_SIZE];
}

NIAbstractTask::~NIAbstractTask()
{
    clear();
    delete[] errBuff;
}

bool NIAbstractTask::isInitialized()
{
    return task != nullptr;
}

bool NIAbstractTask::isTaskDone()
{
    bool32 done = 1;
    DAQmxErrChk(DAQmxIsTaskDone(task, &done));
    return static_cast<bool>(done);
}

void NIAbstractTask::initializeTask()
{
    clear();
#ifdef WITH_HARDWARE
    if (DAQmxFailed(DAQmxCreateTask(getTaskName().toLatin1(), &task))) {
        onError();
        return;
    }
#endif
    initializeTask_impl();
}

void NIAbstractTask::start()
{
    if (!task) {
        initializeTask();
    }
    DAQmxErrChk(DAQmxStartTask(task));
}

void NIAbstractTask::stop()
{
    if (!task) {
        return;
    }
    DAQmxErrChk(DAQmxStopTask(task));
}

void NIAbstractTask::clear()
{
    if (!task) {
        return;
    }
    stop();
    DAQmxErrChk(DAQmxClearTask(task));
    task = nullptr;
}

/**
 * @brief Recovers the string description of the last occurred error and throws
 * a std::runtime_error exception.
 */

void NIAbstractTask::onError() const
{
#ifdef WITH_HARDWARE
    DAQmxGetExtendedErrorInfo(errBuff, ERRBUF_SIZE);
#endif
    logger->error(errBuff);
    throw std::runtime_error(errBuff);
}

NI::int32 NIAbstractTask::getTriggerEdge() const
{
    return triggerEdge;
}

void NIAbstractTask::setTriggerEdge(const NI::int32 &value)
{
    triggerEdge = value;
}

QString NIAbstractTask::getTriggerTerm() const
{
    return triggerTerm;
}

void NIAbstractTask::setTriggerTerm(const QString &value)
{
    triggerTerm = value;
}

void NIAbstractTask::configureTriggering()
{
    if (triggerTerm.isEmpty()) {
        return;
    }
    DAQmxErrChk(
        DAQmxCfgDigEdgeStartTrig(
            task,
            triggerTerm.toLatin1(),
            triggerEdge
            )
        );
}

NI::uInt64 NIAbstractTask::getNSamples() const
{
    return nSamples;
}

void NIAbstractTask::setNSamples(const NI::uInt64 &value)
{
    nSamples = value;
}

double NIAbstractTask::getSampleRate() const
{
    return sampleRate;
}

void NIAbstractTask::setSampleRate(double value)
{
    sampleRate = value;
}

QString NIAbstractTask::getTaskName() const
{
    return taskName;
}

void NIAbstractTask::setTaskName(const QString &value)
{
    taskName = value;
}

void NIAbstractTask::appendToTaskName(const QString &suffix)
{
    setTaskName(taskName + suffix);
}

void NIAbstractTask::configureSampleClockTiming(
    const QString &source,
    NI::int32 activeEdge,
    NI::int32 sampleMode
    )
{
#ifndef WITH_HARDWARE
    Q_UNUSED(source)
    Q_UNUSED(activeEdge)
    Q_UNUSED(sampleMode)
#endif
    DAQmxErrChk(
        DAQmxCfgSampClkTiming(
            task,
            source.isEmpty() ? nullptr : source.toLatin1(),  // clock source (onboard if NULL)
            sampleRate,  // samples per second per channel
            activeEdge,
            sampleMode,
            nSamples
            )
        );
}

QStringList NIAbstractTask::getPhysicalChannels() const
{
    return physicalChannels;
}

void NIAbstractTask::setPhysicalChannels(const QStringList &channels)
{
    physicalChannels = channels;
    clear();
    setPhysicalChannels_impl();
}

QString NIAbstractTask::getPhysicalChannel(const int number) const
{
    return physicalChannels.at(number);
}
