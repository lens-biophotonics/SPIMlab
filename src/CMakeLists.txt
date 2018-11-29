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
    orcaflash.cpp
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
    ${QWT_LIBRARY}
)

if (NOT DEMO_MODE)
    target_link_libraries(SPIMlab
        ${NIDAQMX_LIBRARY}
        ${DCAMAPI_LIBRARY}
    )
endif ()
