set(LIBNAME_STR "NI-DAQmx")
set(NIDAQMX_LIBRARY_NAMES nidaqmx)

get_property(INCLUDE_DIRECTORIES DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

find_library(NIDAQMX_LIBRARY
    NAMES ${NIDAQMX_LIBRARY_NAMES}
    PATHS
    /usr/lib/x86_64-linux-gnu
    /usr/lib
    /usr/local/lib
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}"
)

find_path(NIDAQMX_INCLUDE_DIR NAMES NIDAQmx.h PATHS
    "${INCLUDE_DIRECTORIES}"
    /usr/include
    /usr/local/include
    /usr/local/hamamatsu_dcam/sdk/include/
    "$ENV{LIB_DIR}/include"
    "$ENV{INCLUDE}"
)

if (NIDAQMX_INCLUDE_DIR)
    add_definitions(-DNIDAQMX_HEADERS)
endif ()

if (NIDAQMX_INCLUDE_DIR AND NIDAQMX_LIBRARY)
    set(NIDAQMX_FOUND TRUE)
endif ()

if (NIDAQMX_FOUND)
    message(STATUS "Found NI-DAQmx: ${NIDAQMX_LIBRARY}")
else (NIDAQMX_FOUND)
    if (NIDAQMX_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find ${LIBNAME_STR}")
    else (NIDAQMX_FIND_REQUIRED)
        message(STATUS "Could not find ${LIBNAME_STR}. Headers: ${NIDAQMX_INCLUDE_DIR}")
    endif (NIDAQMX_FIND_REQUIRED)
endif (NIDAQMX_FOUND)
