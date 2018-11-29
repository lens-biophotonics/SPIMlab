/** @file */

/**
 * \defgroup NationalInstruments National Instruments
 * @brief National Instruments related classes.
 * \ingroup Hardware
 */

#ifndef NIDEVICE_H
#define NIDEVICE_H

/**
 * \brief Wraps the call to the given function around some logic for error
 * checking
 *
 * If an error has occurred during the function call, then
 * NIDevice::lastOpWasSuccessful is set to true, false otherwise. Additionally,
 * the NIDevice::onError() slot is called.
 *
 * \param functionCall The function to be called
 */
#define DAQmxErrChk(functionCall) { \
        if (DAQmxFailed(functionCall)) { \
            NIDevice::onError(); \
        } \
}

/**
 * \brief As DAQmxErrChk, but make the function that refers to this macro
 * return false
 */
#define DAQmxErrChkRetFalse(functionCall) { \
        if (DAQmxFailed(functionCall)) { \
            NIDevice::onError(); \
            return false; \
        } \
}
#ifdef NIDAQMX_HEADERS
#include <NIDAQmx.h>
#endif
#include <QStringList>
#include <QObject>

/**
 * @brief The NIDevice class provides common functions for operations with
 * National Instruments drivers.
 *
 * \ingroup NationalInstruments
 */

class NIDevice : public QObject
{
    Q_OBJECT
public:
    explicit NIDevice();
    virtual ~NIDevice();

signals:
    void errorOccurred(QString error);

public slots:

protected slots:
    void onError();

protected:
    QStringList errorList;
    bool lastOpWasSuccessful;
    char errBuff[2048];
};

#endif // NIDEVICE_H
