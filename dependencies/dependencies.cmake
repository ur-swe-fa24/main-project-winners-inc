# Logging with spdlog - we will use this logging library for output.
CPMAddPackage("gh:gabime/spdlog@1.14.1")
# Catch2 Unit Testing - this library will be used for unit testing in the future.
CPMAddPackage("gh:catchorg/Catch2@3.6.0")
# wxwidgets - Depends on the system your are using. Might be better to install wxWidgets into your system, e.g. homebrew, apt.
# CPMAddPackage("gh:wxWidgets/wxWidgets@3.2.5")