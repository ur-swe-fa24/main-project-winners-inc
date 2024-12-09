# tests/CMakeLists.txt

# Enable testing
enable_testing()

# Include the Catch2 package
include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

# Define test executables
add_executable(test_mongoDB test_mongoDB.cpp)
target_link_libraries(test_mongoDB PRIVATE main_proj Catch2::Catch2WithMain)
copy_resources(test_mongoDB)

add_executable(test_AlertSystem test_AlertSystem.cpp)
target_link_libraries(test_AlertSystem PRIVATE main_proj Catch2::Catch2WithMain)
copy_resources(test_AlertSystem)

add_executable(test_mappingSystem test_mappingSystem.cpp)
target_link_libraries(test_mappingSystem PRIVATE main_proj Catch2::Catch2WithMain)
copy_resources(test_mappingSystem)

# add_executable(test_robotMetrics test_robotMetrics.cpp)
# target_link_libraries(test_robotMetrics PRIVATE main_proj Catch2::Catch2WithMain)
# copy_resources(test_robotMetrics)

# add_executable(test_schedulingSystem test_schedulingSystem.cpp)
# target_link_libraries(test_schedulingSystem PRIVATE main_proj Catch2::Catch2WithMain)
# copy_resources(test_schedulingSystem)

# add_executable(test_simulation test_simulation.cpp)
# target_link_libraries(test_simulation PRIVATE main_proj Catch2::Catch2WithMain)
# copy_resources(test_simulation)

# Set include directories for all test executables
foreach(test_target
    test_mongoDB
    test_AlertSystem
    test_mappingSystem
    # test_robotMetrics
    # test_schedulingSystem
    # test_simulation
)
    target_include_directories(${test_target}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/include
    )

    # Link common libraries
    target_link_libraries(${test_target}
        PRIVATE
            mongo::mongocxx_shared
            spdlog::spdlog_header_only
    )

    # Ensure C++17 standard
    target_compile_features(${test_target} PRIVATE cxx_std_17)

    # Discover tests
    catch_discover_tests(${test_target})
endforeach()
