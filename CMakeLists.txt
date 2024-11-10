# Root-Level CMakeLists.txt

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(main_project_winners_inc LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include CPM for package management
include(dependencies/get_cpm.cmake)
include(dependencies/dependencies.cmake)

# Find required packages
find_package(spdlog REQUIRED)
find_package(mongocxx REQUIRED)
find_package(Catch2 REQUIRED)
find_package(wxWidgets REQUIRED COMPONENTS core base)

# Define the main_proj library with all necessary source files
add_library(main_proj
    src/role.cpp
    src/user.cpp
    src/permission.cpp
    src/alert.cpp
    src/Robot.cpp
    src/Room.cpp
    src/alert_system.cpp
    src/MongoDBAdapter.cpp
    src/RobotSimulator.cpp  # Include RobotSimulator source file
)

# Set include directories for main_proj
target_include_directories(main_proj
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

# Link libraries to main_proj
target_link_libraries(main_proj
    PUBLIC
        spdlog::spdlog_header_only
        mongo::mongocxx_shared
)

# Add the app subdirectory where executables are defined
add_subdirectory(app)
add_subdirectory(tests)
