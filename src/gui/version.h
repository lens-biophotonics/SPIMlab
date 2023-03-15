#ifndef VERSION_H
#define VERSION_H

#include <QString>

#ifdef MASTER_SPIM
#define PROGRAM_NAME "SPIMlab"
#endif
#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_ATTRIBUTE ""

#define COMPANY_NAME "LENS"

#define BUILD_DATE __DATE__

extern QString getProgramVersionString(bool includingProgramName = false);
extern QString getProgramName();

#endif // VERSION_H
