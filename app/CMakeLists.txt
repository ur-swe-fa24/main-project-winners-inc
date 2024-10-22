add_executable(testing_app
    testing.cpp    
    src/user.cpp     
    src/alert.cpp 
    src/role.cpp 
    src/permission.cpp)

target_include_directories(testing_app PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_compile_features(testing_app PRIVATE cxx_std_17)