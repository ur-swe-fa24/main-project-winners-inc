# app/CMakeLists.txt

# Executable for the wxWidgets application
add_executable(wx_robot_test
    wx_robot_test.cpp
    ${PROJECT_SOURCE_DIR}/src/RobotManagementFrame.cpp
    ${PROJECT_SOURCE_DIR}/src/LoginDialog.cpp
    ${PROJECT_SOURCE_DIR}/src/AlertDialog.cpp
    ${PROJECT_SOURCE_DIR}/src/map_panel.cpp
    ${PROJECT_SOURCE_DIR}/src/scheduler_panel.cpp
    # Include other source files if needed
)

# Set include directories for wx_robot_test
target_include_directories(wx_robot_test
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

# Link required libraries to wx_robot_test
target_link_libraries(wx_robot_test
    PRIVATE
        main_proj
        ${wxWidgets_LIBRARIES}
        mongo::mongocxx_shared
        spdlog::spdlog_header_only
        nlohmann_json::nlohmann_json
)

# Ensure C++ standard is set
target_compile_features(wx_robot_test PRIVATE cxx_std_17)

# Copy resources for wx_robot_test
copy_resources(wx_robot_test)

# # Executable for testing the simulator (RobotSimulationMain.cpp)
# add_executable(simulator_test
#     RobotSimulationMain.cpp
#     # Include other source files if needed
# )

# # Set include directories for simulator_test
# target_include_directories(simulator_test
#     PRIVATE
#         ${PROJECT_SOURCE_DIR}/include
# )

# # Link required libraries to simulator_test
# target_link_libraries(simulator_test
#     PRIVATE
#         main_proj
#         mongo::mongocxx_shared
#         spdlog::spdlog_header_only
#         nlohmann_json::nlohmann_json
# )

# # Ensure C++ standard is set
# target_compile_features(simulator_test PRIVATE cxx_std_17)

# # Copy resources for simulator_test
# copy_resources(simulator_test)
