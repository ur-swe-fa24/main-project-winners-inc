# # app/CMakeLists.txt

# add_executable(testing_app
#     ${PROJECT_SOURCE_DIR}/app/testing.cpp  # Path to testing.cpp
#     ${PROJECT_SOURCE_DIR}/src/user.cpp     # Correct paths to source files
#     ${PROJECT_SOURCE_DIR}/src/alert.cpp 
#     ${PROJECT_SOURCE_DIR}/src/role.cpp 
#     ${PROJECT_SOURCE_DIR}/src/permission.cpp
#     ${PROJECT_SOURCE_DIR}/src/alert_system.cpp
#     ${PROJECT_SOURCE_DIR}/src/Robot.cpp
#     ${PROJECT_SOURCE_DIR}/src/Room.cpp)


# # Add the include directory
# target_include_directories(testing_app PRIVATE ${PROJECT_SOURCE_DIR}/include)

# # Link main_proj library to testing_app
# target_link_libraries(testing_app PRIVATE main_proj)

# # Set C++ standard
# target_compile_features(testing_app PRIVATE cxx_std_17)

# app/CMakeLists.txt

# # Define hello_mongo executable
# add_executable(hello_mongo
#     hello_mongo.cpp
# )

# # Link libraries (MongoDB, spdlog, and main_proj) to hello_mongo executable
# target_link_libraries(hello_mongo PRIVATE
#     spdlog::spdlog_header_only
#     mongo::mongocxx_shared
#     main_proj
# )

# # Optional: Set include directory if needed
# target_include_directories(hello_mongo PRIVATE ${PROJECT_SOURCE_DIR}/include)


# Define the main_test executable for testing app/main.cpp
# Define the main_test executable for testing app/main.cpp
# app/CMakeLists.txt
# app/CMakeLists.txt
# app/CMakeLists.txt

add_executable(main_test
    main.cpp
    ${PROJECT_SOURCE_DIR}/src/MongoDBAdapter.cpp
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

