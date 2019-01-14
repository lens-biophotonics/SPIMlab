set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)

option(DEMO_MODE "Demo mode: simulate hardware" OFF)

if (NOT DEMO_MODE)
    add_definitions(-DWITH_HARDWARE)
else ()
    message(AUTHOR_WARNING "Configuring in DEMO mode")
endif ()

set(OTHER_FILES
    ../uncrustify.cfg
    ../.gitignore
)

add_custom_target(
    uncrustify
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    SOURCES ${OTHER_FILES}
    COMMAND
    uncrustify -c ../uncrustify.cfg --no-backup *.cpp *.h
)

add_subdirectory(core)
add_subdirectory(gui)
