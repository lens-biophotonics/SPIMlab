#include "niabstracttask.h"
#include "logger.h"

using namespace NI;

static Logger *logger = getLogger("NI");

NIAbstractTask::NIAbstractTask() : QObject()
{
}

NIAbstractTask::~NIAbstractTask()
{
    free(errBuff);
}

bool NIAbstractTask::start()
{
    clear();
    initializeTask();
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
