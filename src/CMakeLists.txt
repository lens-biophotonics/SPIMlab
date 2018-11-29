option(DEMO_MODE "Demo mode" OFF)

if(NOT DEMO_MODE)
    ADD_DEFINITIONS(-DWITH_HARDWARE)
endif()

set(SPIMlab_SRCS
    ../resources.qrc
    main.cpp
    mainwindow.cpp
    version.cpp
    logwidget.cpp
    logger.cpp
    nidevice.cpp
    centralwidget.cpp
    cameradisplay.cpp
    cameratrigger.cpp
    galvoramp.cpp
    logmanager.cpp
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
target_link_libraries(SPIMlab Qt5::Widgets ${QWT_LIBRARY})
