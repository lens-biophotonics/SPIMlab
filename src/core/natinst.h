#ifndef NATINST_H
#define NATINST_H

#include <QStringList>

namespace NI {
#ifdef NIDAQMX_HEADERS
#include <NIDAQmx.h>
#endif
QStringList getDevicesInSystem();
QStringList getDOLines();
QStringList getAOPhysicalChans();
QStringList getCOPhysicalChans();
QStringList getTerminals();
}

#endif // NATINST_H
