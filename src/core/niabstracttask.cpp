#include "niabstracttask.h"
#include "logger.h"

using namespace NI;

static Logger *logger = getLogger("NI");

NIAbstractTask::NIAbstractTask(QObject *parent) : QObject(parent)
{
    errBuff = new char[2048];
}

NIAbstractTask::~NIAbstractTask()
{
    delete[] errBuff;
}

bool NIAbstractTask::isInitialized()
{
    return initialized;
}

bool NIAbstractTask::isTaskDone()
{
#ifdef WITH_HARDWARE
    bool32 done;
    DAQmxErrChk(DAQmxIsTaskDone(task, &done));
    return static_cast<bool>(done);
#else
    return true;
#endif
}

void NIAbstractTask::initializeTask()
{
    clear();
    initializeTask_impl();
}

void NIAbstractTask::start()
{
    if (!initialized) {
        initializeTask();
        initialized = true;
    }
#ifdef WITH_HARDWARE
    DAQmxErrChk(DAQmxStartTask(task));
#endif
}

void NIAbstractTask::stop()
{
#ifdef WITH_HARDWARE
    DAQmxErrChk(DAQmxStopTask(task));
#endif
}

void NIAbstractTask::clear()
{
#ifdef WITH_HARDWARE
    if (task)
        DAQmxErrChk(DAQmxClearTask(task));

#endif
    initialized = false;
}

/**
 * @brief Recovers the string description of the last occurred error and throws
 * a std::runtime_error exception.
 */

void NIAbstractTask::onError()
{
#ifdef WITH_HARDWARE
    DAQmxGetExtendedErrorInfo(errBuff, 2048);
#endif
    logger->error(errBuff);
    throw std::runtime_error(errBuff);
}
