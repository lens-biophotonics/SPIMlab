project(libSPIMlabCore)

find_package(Qt5Core REQUIRED)
find_package(NIDAQmx REQUIRED NOT DEMO_MODE)
find_package(DCAMAPI REQUIRED NOT DEMO_MODE)

if (UNIX)
    add_definitions(-DLINUX)  # needed for DCAM-API
endif ()

include_directories(
    ${DCAMAPI_INCLUDE_DIR}
)

set(SRC
    cameratrigger.cpp
    galvoramp.cpp
    logger.cpp
    logmanager.cpp
    nidevice.cpp
    orcaflash.cpp
    savestackworker.cpp
    spimhub.cpp
    statemachine.cpp
    natinst.cpp
)

add_library(SPIMlabCore SHARED ${SRC})
target_link_libraries(SPIMlabCore
    Qt5::Core
)

if (NOT DEMO_MODE)
    target_link_libraries(SPIMlabCore
        ${NIDAQMX_LIBRARY}
        ${DCAMAPI_LIBRARY}
    )
endif ()
