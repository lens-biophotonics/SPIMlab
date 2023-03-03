find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS RemoteObjects)

set(SPIMlab_slave_SRCS
    ../gui/resources.qrc
    ../gui/main.cpp
    ../gui/mainwindow.cpp
    ../gui/version.cpp
    ../gui/settings.cpp

    ../gui/camerapage.cpp
    ../gui/displayworker.cpp
    ../gui/settingswidget.cpp
    ../gui/progresswidget.cpp

    ../gui/savestackworker.cpp
    ../gui/spim.cpp
)

find_package(QtLab REQUIRED
    Core
    Hamamatsu
    Widgets
)
if(${QT_VERSION_MAJOR} LESS 6)
    qt5_generate_repc(SPIMlab_slave_SRCS
        spim.rep
    SOURCE)
endif()

add_executable(SPIMlab_slave ${SPIMlab_slave_SRCS})
target_include_directories(SPIMlab_slave PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_repc_sources(SPIMlab_slave
        spim.rep
    )
endif()

target_compile_definitions(SPIMlab_slave PRIVATE SLAVE_SPIM)
target_link_libraries(SPIMlab_slave
    stdc++
    QtLab::Core
    QtLab::Hamamatsu
    QtLab::Widgets
    Qt${QT_VERSION_MAJOR}::RemoteObjects
)

