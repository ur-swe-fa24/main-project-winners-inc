cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(main_project_winners_inc LANGUAGES CXX)

# Include CPM for package management
include(dependencies/get_cpm.cmake)
include(dependencies/dependencies.cmake)

# Find MongoDB and spdlog packages
find_package(spdlog REQUIRED)
find_package(mongocxx REQUIRED)
find_package(wxWidgets REQUIRED COMPONENTS core base)

# Define the sprint1 library with all necessary source files
add_library(sprint1
    src/role.cpp
    src/user.cpp
    src/permission.cpp
    src/alert.cpp
    src/Robot.cpp
    src/Room.cpp
    src/alert_system.cpp  # Include alert_system.cpp
)

# Set include directories for sprint1
target_include_directories(sprint1
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

# Link MongoDB and spdlog to the sprint1 library
target_link_libraries(sprint1
    PUBLIC
        spdlog::spdlog_header_only
        mongo::mongocxx_shared
)

# Add the app subdirectory where main_test is defined
add_subdirectory(app)
