# spdlog - logging library
CPMAddPackage("gh:gabime/spdlog@1.14.1")

# Catch2 - unit testing
CPMAddPackage("gh:catchorg/Catch2@3.6.0")

find_package(wxWidgets REQUIRED)
include(${wxWidgets_USE_FILE})

# CPMAddPackage("gh:mongodb/mongo-cxx-driver@r3.6.7")
# # MongoDB C++ Driver - find the installed MongoDB driver via system packages
# find_package(libmongocxx REQUIRED)
# find_package(libbsoncxx REQUIRED)

# target_link_libraries(your_target_name PRIVATE spdlog Catch2::Catch2 libmongocxx libbsoncxx ${wxWidgets_LIBRARIES})
