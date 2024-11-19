# tests/CMakeLists.txt

# Enable testing
enable_testing()

# Include the Catch2 package
include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

# Copy test data files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_map.json
              ${CMAKE_CURRENT_BINARY_DIR}/test_map.json
              COPYONLY)

# Define test executables
add_executable(test_mongoDB test_mongoDB.cpp)
add_executable(test_AlertSystem test_AlertSystem.cpp)
add_executable(test_mappingSystem test_mappingSystem.cpp)
add_executable(test_robotMetrics test_robotMetrics.cpp)
add_executable(test_schedulingSystem test_schedulingSystem.cpp)
add_executable(test_simulation test_simulation.cpp)

# Set include directories for all test executables
foreach(test_target
    test_mongoDB
    test_AlertSystem
    test_mappingSystem
    test_robotMetrics
    test_schedulingSystem
    test_simulation
)
    target_include_directories(${test_target}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/include
    )

    # Link common libraries
    target_link_libraries(${test_target}
        PRIVATE
            main_proj
            Catch2::Catch2WithMain
            mongo::mongocxx_shared
            spdlog::spdlog_header_only
    )

    # Ensure C++17 standard
    target_compile_features(${test_target} PRIVATE cxx_std_17)

    # Discover tests
    catch_discover_tests(${test_target})
endforeach()
