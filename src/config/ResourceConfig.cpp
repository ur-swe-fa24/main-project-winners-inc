#include "config/ResourceConfig.hpp"
#include <filesystem>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <iostream>

namespace config {
    std::string ResourceConfig::resourceDir_;
    const std::string ResourceConfig::DEFAULT_MAP_NAME = "map.json";

    void ResourceConfig::initialize(const std::string& resourceDir) {
        if (!resourceDir.empty()) {
            resourceDir_ = resourceDir;
            return;
        }

        // Get the executable directory
        wxStandardPaths& paths = wxStandardPaths::Get();
        wxString exeDir = wxFileName(paths.GetExecutablePath()).GetPath();
        
        // Get the project root directory (3 levels up from executable in build/app/)
        wxString projectRoot = wxFileName(exeDir + "/../..").GetFullPath();
        
        // Check possible resource locations in priority order
        std::vector<std::string> possiblePaths = {
            (exeDir + "/resources").ToStdString(),          // build/app/resources
            (projectRoot + "/resources").ToStdString(),     // project_root/resources
            (exeDir + "/..").ToStdString(),                // build/app
            (projectRoot).ToStdString(),                   // project_root
            (exeDir).ToStdString()                        // build/app
        };

        std::cout << "Searching for resources in the following locations:" << std::endl;
        for (const auto& path : possiblePaths) {
            std::cout << "Checking: " << path << std::endl;
            std::string mapPath = path + "/" + DEFAULT_MAP_NAME;
            if (std::filesystem::exists(mapPath)) {
                std::cout << "Found map.json at: " << mapPath << std::endl;
                resourceDir_ = std::filesystem::path(path).string();
                return;
            }
        }

        // If still not found, try current directory
        if (resourceDir_.empty()) {
            std::string currentDir = std::filesystem::current_path().string();
            std::cout << "Falling back to current directory: " << currentDir << std::endl;
            resourceDir_ = currentDir;
        }
    }

    std::string ResourceConfig::getResourcePath(const std::string& resourceName) {
        if (resourceDir_.empty()) {
            initialize();
        }
        return (std::filesystem::path(resourceDir_) / resourceName).string();
    }

    std::string ResourceConfig::getMapPath() {
        return getResourcePath(DEFAULT_MAP_NAME);
    }
}
