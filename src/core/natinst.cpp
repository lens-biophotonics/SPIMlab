#include "logger.h"
#include "natinst.h"

using namespace NI;

static Logger *logger = getLogger("NI");

QStringList NI::getDevicesInSystem()
{
    QStringList list;
#ifdef WITH_HARDWARE
    char buf[2048];
    if (DAQmxFailed(DAQmxGetSysDevNames(buf, 2048))) {
        DAQmxGetExtendedErrorInfo(buf, 2048);
        logger->error(buf);
        return list;
    }
    list = QString(buf).split(", ");
#endif
    return list;
}

#ifdef WITH_HARDWARE
typedef int32 (*daqmx_f_ptr)(const char*, char*, uInt32);

static QStringList getList(daqmx_f_ptr myfp)
{
    QStringList list;
    char buf[2048];
    QStringList devList = NI::getDevicesInSystem();
    QStringListIterator devIt(devList);
    while (devIt.hasNext()) {
        QString dev = devIt.next();
        if (DAQmxFailed(myfp(dev.toLatin1(), buf, 2048))) {
            DAQmxGetExtendedErrorInfo(buf, 2048);
            logger->error(buf);
            return list;
        };
        QStringList cl = QString(buf).split(", ");
        list.append(cl);
    }
    return list;
}
#endif

#ifdef WITH_HARDWARE
#define DEF_NI_GET_FUNCTION(funcName, DAQmxFuncName) \
    QStringList NI::funcName() { \
        return getList(&DAQmxFuncName); \
    }
#else
#define DEF_NI_GET_FUNCTION(funcName, DAQmxFuncNAme) \
    QStringList NI::funcName() { \
        return QStringList(); \
    }
#endif

DEF_NI_GET_FUNCTION(getDOLines, DAQmxGetDevDOLines)
DEF_NI_GET_FUNCTION(getAOPhysicalChans, DAQmxGetDevAOPhysicalChans)
DEF_NI_GET_FUNCTION(getCOPhysicalChans, DAQmxGetDevCOPhysicalChans)
DEF_NI_GET_FUNCTION(getTerminals, DAQmxGetDevTerminals)
