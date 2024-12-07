#include "config/ResourceConfig.hpp"
#include <filesystem>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <iostream>

namespace config {
    std::string ResourceConfig::resourceDir_;
    const std::string ResourceConfig::DEFAULT_MAP_NAME = "map.json";

    bool ResourceConfig::initialize(const std::string& resourceDir) {
        if (!resourceDir.empty()) {
            resourceDir_ = resourceDir;
            return std::filesystem::exists(resourceDir_);
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

        // Try each path until we find one that exists and contains map.json
        for (const auto& path : possiblePaths) {
            if (std::filesystem::exists(path)) {
                std::string mapPath = path + "/" + DEFAULT_MAP_NAME;
                if (std::filesystem::exists(mapPath)) {
                    resourceDir_ = path;
                    std::cout << "Found resource directory at: " << resourceDir_ << std::endl;
                    return true;
                }
            }
        }

        std::cerr << "Failed to find valid resource directory containing " << DEFAULT_MAP_NAME << std::endl;
        return false;
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
