#pragma once

#include <string>
#include <filesystem>

namespace config {
    class ResourceConfig {
    public:
        static void initialize(const std::string& resourceDir = "");
        static std::string getResourcePath(const std::string& resourceName);
        static std::string getMapPath();
        
    private:
        static std::string resourceDir_;
        static const std::string DEFAULT_MAP_NAME;
    };
}
