set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)

option(DEMO_MODE "Demo mode: simulate hardware" OFF)

if (NOT DEMO_MODE)
    add_definitions(-DWITH_HARDWARE)
else ()
    message(AUTHOR_WARNING "Configuring in DEMO mode")
endif ()

find_package(Qt5Widgets REQUIRED)
find_package(Qwt REQUIRED)
find_package(NIDAQmx REQUIRED NOT DEMO_MODE)
find_package(DCAMAPI REQUIRED NOT DEMO_MODE)
find_package(Boost REQUIRED)


include_directories(
    ${Qwt_INCLUDE_DIR}
    ${DCAMAPI_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
)

if (UNIX)
    add_definitions(-DLINUX)  # needed for DCAM-API
endif ()

set(SPIMlab_SRCS
    ../resources.qrc
    main.cpp
    mainwindow.cpp
    version.cpp
    logwidget.cpp
    logger.cpp
    nidevice.cpp
    centralwidget.cpp
    cameraplot.cpp
    cameradisplay.cpp
    cameratrigger.cpp
    galvoramp.cpp
    logmanager.cpp
    orcaflash.cpp
    spimevent.cpp
)

set(OTHER_FILES
    ../uncrustify.cfg
    ../.gitignore
)

add_custom_target(
    uncrustify
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    uncrustify -c ../uncrustify.cfg --no-backup *.cpp *.h
)

add_executable(SPIMlab ${SPIMlab_SRCS} ${OTHER_FILES})
target_link_libraries(SPIMlab
    Qt5::Widgets
    ${Qwt_LIBRARY}
)

if (NOT DEMO_MODE)
    target_link_libraries(SPIMlab
        ${NIDAQMX_LIBRARY}
        ${DCAMAPI_LIBRARY}
    )
endif ()
