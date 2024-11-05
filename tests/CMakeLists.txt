find_package(Catch2 3 REQUIRED)

if (Catch2_FOUND)
    message(STATUS "Found Catch2")
else ()
    message(FATAL_ERROR "Could not find Catch2.")
endif ()

add_executable(test_mongoDB test_mongoDB.cpp)


target_include_directories(main_test
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

# Ensure linking to main_proj and other dependencies
target_link_libraries(test_mongoDB
    PRIVATE
        Catch2::Catch2WithMain
        main_proj
        mongo::mongocxx_shared
        spdlog::spdlog_header_only
)

# Ensure C++ standard is set
target_compile_features(test_mongoDB PRIVATE cxx_std_17)
