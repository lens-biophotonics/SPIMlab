#ifndef VERSION_H
#define VERSION_H

#include <QString>

#define PROGRAM_NAME "SPIMlab"
#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_ATTRIBUTE ""

#define COMPANY_NAME "LENS"

#define BUILD_DATE __DATE__

extern QString getProgramVersionString(bool includingProgramName=false);
extern QString getProgramName();

#endif // VERSION_H
