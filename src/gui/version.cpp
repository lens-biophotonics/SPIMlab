#include "version.h"

QString getProgramVersionString(bool includingProgramName)
{
    QString ret = QString("%1.%2.%3")
                  .arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_PATCH);
    if (!QString(VERSION_ATTRIBUTE).isEmpty())
        ret.append(QString(" %1").arg(VERSION_ATTRIBUTE));
    ret.append(" (built ");
    ret.append(BUILD_DATE);
    ret.append(")");
    if (includingProgramName) {
        ret.prepend(" ");
        ret.prepend(getProgramName());
    }
    return ret;
}

QString getProgramName()
{
    return QString("%1").arg(PROGRAM_NAME);
}
