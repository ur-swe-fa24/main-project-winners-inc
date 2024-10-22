# app/CMakeLists.txt

add_executable(testing_app
    ${PROJECT_SOURCE_DIR}/app/testing.cpp  # Path to testing.cpp
    ${PROJECT_SOURCE_DIR}/src/user.cpp     # Correct paths to source files
    ${PROJECT_SOURCE_DIR}/src/alert.cpp 
    ${PROJECT_SOURCE_DIR}/src/role.cpp 
    ${PROJECT_SOURCE_DIR}/src/permission.cpp
    ${PROJECT_SOURCE_DIR}/src/alert_system.cpp
    ${PROJECT_SOURCE_DIR}/src/Robot.cpp
    ${PROJECT_SOURCE_DIR}/src/Room.cpp)

# Add the include directory
target_include_directories(testing_app PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Link sprint1 library to testing_app
target_link_libraries(testing_app PRIVATE sprint1)

# Set C++ standard
target_compile_features(testing_app PRIVATE cxx_std_17)