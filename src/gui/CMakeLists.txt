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
)
