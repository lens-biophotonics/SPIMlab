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

bool NIAbstractTask::initializeTask()
{
    clear();
    return initializeTask_impl();
}

bool NIAbstractTask::start()
{
    if (!initialized) {
        if (!initializeTask()) {
            return false;
        }
        initialized = true;
    }
#ifdef WITH_HARDWARE
    DAQmxErrChkRetFalse(DAQmxStartTask(task));
#endif
    return true;
}

bool NIAbstractTask::stop()
{
#ifdef WITH_HARDWARE
    DAQmxErrChkRetFalse(DAQmxStopTask(task));
#endif
    return true;
}

bool NIAbstractTask::clear()
{
#ifdef WITH_HARDWARE
    if (task)
        DAQmxErrChkRetFalse(DAQmxClearTask(task));

#endif
    initialized = false;
    return true;
}

/**
 * @brief Recovers the string description of the last occurred error and emits
 * the error() signal.
 */

void NIAbstractTask::onError()
{
#ifdef WITH_HARDWARE
    DAQmxGetExtendedErrorInfo(errBuff, 2048);
#endif
    logger->error(errBuff);
    emit error();
}
