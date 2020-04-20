set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)

find_package(Qt5 5.8 REQUIRED COMPONENTS
    Widgets
    SerialPort
)
find_package(QtLab REQUIRED Core Widgets NI PI Serial Hamamatsu)

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

    controlwidget.cpp
    picontrollersettingswidget.cpp
    nisettingswidget.cpp
    galvowaveformwidget.cpp
    pipositioncontrolwidget.cpp
    coboltwidget.cpp
    filterwheelwidget.cpp
    acquisitionwidget.cpp
    progresswidget.cpp

    centralwidget.cpp
    camerapage.cpp
    displayworker.cpp
    laserpage.cpp
    settingspage.cpp

    utils.cpp
    
    cameratrigger.cpp
    galvoramp.cpp
    
    savestackworker.cpp
    spim.cpp
)

add_executable(SPIMlab ${SPIMlab_SRCS})
target_link_libraries(SPIMlab
    stdc++
    Qt5::Widgets
    Qt5::SerialPort
    QtLab::Core
    QtLab::Widgets
    QtLab::NI
    QtLab::PI
    QtLab::Hamamatsu
    QtLab::Serial
)
