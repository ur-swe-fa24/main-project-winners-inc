cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(sprint2 LANGUAGES CXX)

include(dependencies/get_cpm.cmake)
include(dependencies/dependencies.cmake)

add_library(sprint2
    src/role.cpp
    src/user.cpp
    src/permission.cpp
    src/alert.cpp
    src/alert_system.cpp
    src/Robot.cpp
    src/Room.cpp)

target_include_directories(sprint1
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

add_subdirectory(app)
