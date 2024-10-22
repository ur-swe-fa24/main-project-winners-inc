cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(sprint1 LANGUAGES CXX)

include(dependencies/get_cpm.cmake)
include(dependencies/dependencies.cmake)

add_library(sprint1
    src/role.cpp
    src/user.cpp
    src/permission.cpp
    src/alert.cpp)

target_include_directories(sprint1
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

add_subdirectory(app)