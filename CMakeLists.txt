# Root-Level CMakeLists.txt

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(main_project_winners_inc LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include CPM for package management
include(dependencies/get_cpm.cmake)
include(dependencies/dependencies.cmake)  # Ensure this includes CPMAddPackage for nlohmann_json

# Find required packages
find_package(spdlog REQUIRED)
find_package(mongocxx REQUIRED)
find_package(Catch2 3 REQUIRED)
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

# Enable testing
enable_testing()

# Define the main_proj library with all necessary source files
add_library(main_proj
    src/role.cpp
    src/user.cpp
    src/permission.cpp
    src/alert.cpp
    src/Robot.cpp
    src/Room.cpp
    src/virtual_wall.cpp
    src/map.cpp
    src/alert_system.cpp
    src/MongoDBAdapter.cpp
    src/RobotSimulator.cpp
    src/Schedular.cpp
    src/robot_metrics.cpp
    src/config/ResourceConfig.cpp
    # Include other source files if needed
)

# Set include directories for main_proj
target_include_directories(main_proj
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries to main_proj
target_link_libraries(main_proj
    PUBLIC
        spdlog::spdlog_header_only
        mongo::mongocxx_shared
        nlohmann_json::nlohmann_json
        ${wxWidgets_LIBRARIES}
)

# Set up resources directory
set(RESOURCES_DIR ${CMAKE_SOURCE_DIR}/resources)
set(MAP_FILE ${RESOURCES_DIR}/map.json)

# Function to copy resources to target directory
function(copy_resources TARGET_NAME)
    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${RESOURCES_DIR}
            $<TARGET_FILE_DIR:${TARGET_NAME}>/resources
        COMMENT "Copying resources for ${TARGET_NAME}"
    )
endfunction()

# Add subdirectories
add_subdirectory(app)
add_subdirectory(tests)
