project(libSPIMlabCore)

find_package(Qt5Core REQUIRED)
find_package(NIDAQmx REQUIRED NOT DEMO_MODE)
find_package(DCAMAPI REQUIRED NOT DEMO_MODE)
find_package(PIGCS REQUIRED)

if (UNIX)
    add_definitions(-DLINUX)  # needed for DCAM-API
endif ()

include_directories(
    ${DCAMAPI_INCLUDE_DIR}
)

set(SRC
    # National Instruments
    natinst.cpp
    niabstracttask.cpp
    cameratrigger.cpp
    galvoramp.cpp

    # Hamamatsu
    dcamutils.cpp
    orcaflash.cpp

    # PI
    pidaisychain.cpp
    pidevice.cpp

    # Cobolt
    cobolt.cpp
    serialport.cpp

    logger.cpp
    logmanager.cpp
    savestackworker.cpp
    spim.cpp
)

add_library(SPIMlabCore SHARED ${SRC})
target_link_libraries(SPIMlabCore
    Qt5::Core
    ${PIGCS_LIBRARY}
)
qt5_use_modules(SPIMlabCore Core SerialPort)


if (NOT DEMO_MODE)
    target_link_libraries(SPIMlabCore
        ${NIDAQMX_LIBRARY}
        ${DCAMAPI_LIBRARY}
    )
endif ()
