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

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS RemoteObjects)

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
    cameradelaywidget.cpp

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

if(${QT_VERSION_MAJOR} LESS 6)
    qt5_generate_repc(SPIMlab_SRCS
        ../slave/spim.rep
    REPLICA)
endif()

add_executable(SPIMlab ${SPIMlab_SRCS})
target_include_directories(SPIMlab PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_repc_replicas(SPIMlab
        spim.rep
    )
endif()

target_compile_definitions(SPIMlab PRIVATE MASTER_SPIM)
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
    Qt${QT_VERSION_MAJOR}::RemoteObjects
)
