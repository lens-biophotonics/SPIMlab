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
