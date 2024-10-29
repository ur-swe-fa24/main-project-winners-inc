# cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# project(sprint1 LANGUAGES CXX)

# include(dependencies/get_cpm.cmake)
# include(dependencies/dependencies.cmake)

# # Define the sprint1 library
# add_library(sprint1
#     src/role.cpp
#     src/user.cpp
#     src/permission.cpp
#     src/alert.cpp
#     src/robot.cpp       # Add all source files
#     src/room.cpp)

# # Set the include directory for sprint1
# target_include_directories(sprint1
#     PUBLIC
#         $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> # Make sure it points to the right directory
# )
# add_subdirectory(app)
# Root CMakeLists.txt
# Root CMakeLists.txt

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

# Define the sprint1 library
add_library(sprint1
    src/role.cpp
    src/user.cpp
    src/permission.cpp
    src/alert.cpp
    src/robot.cpp
    src/room.cpp
)

# Set include directory for sprint1
target_include_directories(sprint1
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

# Link MongoDB and spdlog to sprint1 library
target_link_libraries(sprint1
    PUBLIC
        spdlog::spdlog_header_only
        mongo::mongocxx_shared
)

# Add the app subdirectory where hello_mongo is defined
add_subdirectory(app)
