#ifndef DCAMUTILS_H
#define DCAMUTILS_H

namespace DCAM {
#ifdef DCAMAPI_HEADERS
#include <dcamapi.h>
#include <dcamprop.h>
#endif

int init_dcam();
bool uninit_dcam();
}

#endif // DCAMUTILS_H
