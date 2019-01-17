/** @file */

/**
 * \defgroup NationalInstruments National Instruments
 * @brief National Instruments related classes.
 * \ingroup Hardware
 */

#ifndef NIABSTRACTTASK_H
#define NIABSTRACTTASK_H

/**
 * \brief Wraps the call to the given function around some logic for error
 * checking
 *
 * If an error has occurred during the function call, NIAbstractTask::onError()
 * is called.
 *
 * \param functionCall The function to be called
 */
#define DAQmxErrChk(functionCall) { \
        if (DAQmxFailed(functionCall)) { \
            onError(); \
        } \
}

#include <QObject>

#include "natinst.h"

/**
 * @brief The NIAbstractTask class is an abstract wrapper for a DAQmx Task.
 *
 * \ingroup NationalInstruments
 */

class NIAbstractTask : public QObject
{
    Q_OBJECT
public:
    explicit NIAbstractTask(QObject *parent = nullptr);
    virtual ~NIAbstractTask();

    bool isInitialized();
    bool isTaskDone();

signals:
    void error();

public slots:
    void initializeTask();
    void start();
    void stop();
    void clear();

protected:
    [[ noreturn ]] void onError();
#ifdef NIDAQMX_HEADERS
    NI::TaskHandle task = nullptr;
#endif

private:
    virtual void initializeTask_impl() = 0;

    char *errBuff;
};

#endif // NIABSTRACTTASK_H
