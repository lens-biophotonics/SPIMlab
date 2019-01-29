find_package(Qt5Widgets REQUIRED)
find_package(Qwt REQUIRED)
find_package(NIDAQmx REQUIRED NOT DEMO_MODE)
find_package(DCAMAPI REQUIRED NOT DEMO_MODE)

include_directories(
    ${Qwt_INCLUDE_DIR}
    ${DCAMAPI_INCLUDE_DIR}
    ..
)

set(SPIMlab_SRCS
    resources.qrc
    main.cpp
    mainwindow.cpp
    version.cpp
    logwidget.cpp
    controlwidget.cpp
    settingswidget.cpp
    centralwidget.cpp
    cameraplot.cpp
    cameradisplay.cpp
)

add_executable(SPIMlab ${SPIMlab_SRCS})
target_link_libraries(SPIMlab
    stdc++
    Qt5::Widgets
    ${Qwt_LIBRARY}
    SPIMlabCore
)
