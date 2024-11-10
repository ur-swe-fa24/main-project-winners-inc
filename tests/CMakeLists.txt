# tests/CMakeLists.txt

# Enable testing
enable_testing()

# Include the Catch2 package
include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

# Define test executables
add_executable(test_mongoDB test_mongoDB.cpp)
add_executable(test_AlertSystem test_AlertSystem.cpp)

# Set include directories for test executables
target_include_directories(test_mongoDB
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_include_directories(test_AlertSystem
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

# Link libraries to test executables
target_link_libraries(test_mongoDB
    PRIVATE
        main_proj
        Catch2::Catch2WithMain
        mongo::mongocxx_shared
        spdlog::spdlog_header_only
)

target_link_libraries(test_AlertSystem
    PRIVATE
        main_proj
        Catch2::Catch2WithMain
        mongo::mongocxx_shared
        spdlog::spdlog_header_only
)

# Ensure C++ standard is set
target_compile_features(test_mongoDB PRIVATE cxx_std_17)
target_compile_features(test_AlertSystem PRIVATE cxx_std_17)

# Discover tests
catch_discover_tests(test_mongoDB)
catch_discover_tests(test_AlertSystem)
