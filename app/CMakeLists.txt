# app/CMakeLists.txt

# Executable for testing the main application
add_executable(main_test
    main.cpp
    # Include other source files if needed
)

# Set include directories for main_test
target_include_directories(main_test
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

# Link main_test to main_proj, MongoDB, and spdlog
target_link_libraries(main_test
    PRIVATE
        main_proj
        mongo::mongocxx_shared
        spdlog::spdlog_header_only
)

# Ensure C++ standard is set for main_test
target_compile_features(main_test PRIVATE cxx_std_17)

# Executable for the wxWidgets application
add_executable(wx_robot_test
    wx_robot_test.cpp
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
)

# Ensure C++ standard is set
target_compile_features(wx_robot_test PRIVATE cxx_std_17)

# Executable for testing the simulator (simulatormain.cpp)
add_executable(simulator_test
    simulatormain.cpp
    # Include other source files if needed
)

# Set include directories for simulator_test
target_include_directories(simulator_test
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

# Link required libraries to simulator_test
target_link_libraries(simulator_test
    PRIVATE
        main_proj
        mongo::mongocxx_shared
        spdlog::spdlog_header_only
)

# Ensure C++ standard is set
target_compile_features(simulator_test PRIVATE cxx_std_17)
