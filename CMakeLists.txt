cmake_minimum_required(VERSION 3.1.0)

project(SPIMlab)

include_directories(${INCLUDE_DIRS})
message(STATUS "Custom include directories: ${INCLUDE_DIRS}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(OTHER_FILES
    .gitignore
    .clang-format
    LICENSE.txt
    README.md
)

add_custom_target(
    clang-format
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    SOURCES .clang-format
    COMMAND
    clang-format -i src/gui/*.cpp src/gui/*.h
)

add_custom_target(project-related-files SOURCES ${OTHER_FILES})

option(DEMO_MODE "Demo mode" OFF)

if(DEMO_MODE)
    add_definitions(-DDEMO_MODE)
endif()

option(FORCE_FUSION_STYLE "Force fusion style" OFF)

if (FORCE_FUSION_STYLE)
    add_definitions(-DFORCE_FUSION_STYLE)
    message(AUTHOR_WARNING "Forcing fusion style")
endif()

option(BUILD_MASTER_SPIM "Build master SPIM" ON)

if(BUILD_MASTER_SPIM)
    add_subdirectory(src/gui)
endif()
