find_package(Qt5Widgets 5.8 REQUIRED)
find_package(Qwt REQUIRED)
find_package(NIDAQmx REQUIRED NOT DEMO_MODE)
find_package(DCAMAPI REQUIRED NOT DEMO_MODE)

include_directories(
    ${Qwt_INCLUDE_DIR}
    ${DCAMAPI_INCLUDE_DIR}
    ..
)

option(FORCE_FUSION_STYLE "Force fusion style" OFF)

if (FORCE_FUSION_STYLE)
    add_definitions(-DFORCE_FUSION_STYLE)
    message(AUTHOR_WARNING "Forcing fusion style")
endif ()

set(SPIMlab_SRCS
    resources.qrc
    main.cpp
    mainwindow.cpp
    version.cpp
    settings.cpp

    aspectratiowidget.cpp
    customspinbox.cpp

    controlwidget.cpp
    picontrollersettingswidget.cpp
    nisettingswidget.cpp
    galvowaveformwidget.cpp
    pipositioncontrolwidget.cpp
    coboltwidget.cpp
    acquisitionwidget.cpp
    progresswidget.cpp

    centralwidget.cpp
    camerapage.cpp
    coboltpage.cpp
    settingspage.cpp
    logwidget.cpp
    cameradisplay.cpp

    cameraplot.cpp
    colormaps.cpp

    utils.cpp
)

add_executable(SPIMlab ${SPIMlab_SRCS})
target_link_libraries(SPIMlab
    stdc++
    Qt5::Widgets
    ${Qwt_LIBRARY}
    SPIMlabCore
)
