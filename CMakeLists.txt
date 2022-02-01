cmake_minimum_required(VERSION 3.1.0)

project(SPIMlab)

include_directories(${INCLUDE_DIRS})
message(STATUS "Custom include directories: ${INCLUDE_DIRS}")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(OTHER_FILES
    .gitignore
    .clang-format
    LICENSE.txt
)

add_custom_target(
    clang-format
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    SOURCES .clang-format
    COMMAND
    clang-format -i src/gui/*.cpp src/gui/*.h
)

add_custom_target(project-related-files SOURCES ${OTHER_FILES})

add_subdirectory(src/gui)
