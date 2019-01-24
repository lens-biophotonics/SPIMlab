set(LIBNAME_STR "PI GCS2")
set(PIGCS_LIBRARY_NAMES pi_pi_gcs2)

get_property(INCLUDE_DIRECTORIES DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

find_library(PIGCS_LIBRARY
    NAMES ${PIGCS_LIBRARY_NAMES}
    PATHS
    /usr/lib/x86_64-linux-gnu
    /usr/lib
    /usr/local/lib
    /usr/local/PI
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}"
)

find_path(PIGCS_INCLUDE_DIR NAMES PI_GCS2_DLL.h PATHS
    "${INCLUDE_DIRECTORIES}"
    /usr/local/PI/include
    /usr/include
    /usr/local/include
    "$ENV{LIB_DIR}/include"
    "$ENV{INCLUDE}"
)

if (PIGCS_INCLUDE_DIR AND PIGCS_LIBRARY)
    set(PIGCS_FOUND TRUE)
endif ()

if (PIGCS_FOUND)
    message(STATUS "Found ${LIBNAME_STR}: ${PIGCS_LIBRARY}")
else (PIGCS_FOUND)
    if (PIGCS_FIND_REQUIRED)
        message(STATUS "Could not find ${LIBNAME_STR}. Headers: ${PIGCS_INCLUDE_DIR}")
    endif (PIGCS_FIND_REQUIRED)
endif ()
