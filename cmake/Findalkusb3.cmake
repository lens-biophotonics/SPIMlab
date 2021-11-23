set(ALKUSB3_LIBNAME_STR "libalkusb3")
set(ALKUSB3_LIBRARY_NAMES alkusb3)

find_library(ALKUSB3_LIBRARY
    NAMES ${ALKUSB3_LIBRARY_NAMES}
    PATHS
    /usr/lib
    /usr/local/lib
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}"
)

find_path(ALKUSB3_INCLUDE_DIR NAMES ICeleraCamera.h PATHS
    "${INCLUDE_DIRS}"
    /usr/include
    /usr/local/include
    /usr/local/include/libalkusb3
    "$ENV{LIB_DIR}/include"
    "$ENV{INCLUDE}"
)

if (ALKUSB3_INCLUDE_DIR)
    message(STATUS "Found ${ALKUSB3_LIBNAME_STR} headers in: ${ALKUSB3_INCLUDE_DIR}")
endif ()

if (ALKUSB3_INCLUDE_DIR AND ALKUSB3_LIBRARY)
    set(ALKUSB3_FOUND TRUE)
    message(STATUS "Found ${ALKUSB3_LIBNAME_STR}: ${ALKUSB3_LIBRARY}")
endif ()

if (NOT ALKUSB3_FOUND AND ALKUSB3_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find ${ALKUSB3_LIBNAME_STR}")
endif ()
