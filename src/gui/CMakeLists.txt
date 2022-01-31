set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)

find_package(Qt5 5.8 REQUIRED COMPONENTS
    Widgets
    SerialPort
)
find_package(QtLab REQUIRED
    Core
    Hamamatsu
    NI
    PI
    PI-Widgets
    Serial
    Serial-Widgets
    Widgets
)

option(FORCE_FUSION_STYLE "Force fusion style" OFF)

if (FORCE_FUSION_STYLE)
    add_definitions(-DFORCE_FUSION_STYLE)
    message(AUTHOR_WARNING "Forcing fusion style")
endif ()

option(DEMO_MODE "Demo mode" OFF)

if(DEMO_MODE)
    add_definitions(-DDEMO_MODE)
endif()

set(SPIMlab_SRCS
    resources.qrc
    main.cpp
    mainwindow.cpp
    version.cpp
    settings.cpp

    nisettingswidget.cpp
    galvowaveformwidget.cpp
    coboltwidget.cpp
    filterwheelwidget.cpp
    acquisitionwidget.cpp
    progresswidget.cpp

    stagewidget.cpp
    camerapage.cpp
    displayworker.cpp
    filterswidget.cpp
    settingswidget.cpp

    utils.cpp
    
    cameratrigger.cpp
    galvoramp.cpp
    tasks.cpp
    
    savestackworker.cpp
    spim.cpp
)

add_executable(SPIMlab ${SPIMlab_SRCS})
target_link_libraries(SPIMlab
    stdc++
    QtLab::Core
    QtLab::Hamamatsu
    QtLab::NI
    QtLab::PI
    QtLab::PI-Widgets
    QtLab::Serial
    QtLab::Serial-Widgets
    QtLab::Widgets
)
