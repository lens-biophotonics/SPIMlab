cmake_minimum_required(VERSION 3.1.0)

project(SPIMlab)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qwt REQUIRED)

add_subdirectory(src)
