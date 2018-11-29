set(LIBNAME_STR "Hamamatsu DCAM-API")
set(DCAMAPI_LIBRARY_NAMES dcamapi)

get_property(INCLUDE_DIRECTORIES DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

find_library(DCAMAPI_LIBRARY
    NAMES ${DCAMAPI_LIBRARY_NAMES}
    PATHS
    /usr/lib
    /usr/local/lib
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}"
)

find_path(DCAMAPI_INCLUDE_DIR NAMES dcamapi.h PATHS
    "${INCLUDE_DIRECTORIES}"
    /usr/include
    /usr/local/include
    /usr/local/hamamatsu_dcam/sdk/include/
    "$ENV{LIB_DIR}/include"
    "$ENV{INCLUDE}"
)

if (DCAMAPI_INCLUDE_DIR)
    add_definitions(-DDCAMAPI_HEADERS)
endif ()

if (DCAMAPI_INCLUDE_DIR AND DCAMAPI_LIBRARY)
    set(DCAMAPI_FOUND TRUE)
endif ()

if (DCAMAPI_INCLUDE_DIR AND DEMO_MODE)
    set(DCAMAPI_FOUND TRUE)
    message(STATUS "Found ${LIBNAME_STR} headers in: ${DCAMAPI_INCLUDE_DIR}")
elseif (DCAMAPI_FOUND)
    message(STATUS "Found ${LIBNAME_STR}: ${DCAMAPI_LIBRARY}")
else (DCAMAPI_FOUND)
    if (DCAMAPI_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find ${LIBNAME_STR}")
    endif (DCAMAPI_FIND_REQUIRED)
endif (DCAMAPI_INCLUDE_DIR AND DEMO_MODE)
