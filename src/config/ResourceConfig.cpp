#include "config/ResourceConfig.hpp"
#include <filesystem>
#include <wx/filename.h>
#include <wx/stdpaths.h>

namespace config {
    std::string ResourceConfig::resourceDir_;
    const std::string ResourceConfig::DEFAULT_MAP_NAME = "map.json";

    void ResourceConfig::initialize(const std::string& resourceDir) {
        if (!resourceDir.empty()) {
            resourceDir_ = resourceDir;
        } else {
            // Try to find the resource directory relative to the executable
            wxStandardPaths& paths = wxStandardPaths::Get();
            wxString exeDir = wxFileName(paths.GetExecutablePath()).GetPath();
            
            // Check possible resource locations
            std::vector<std::string> possiblePaths = {
                (exeDir + "/resources").ToStdString(),  // resources in exe directory
                (exeDir + "/map.json").ToStdString(),   // map.json in exe directory
                (exeDir + "/../resources").ToStdString(), // resources one level up
                (exeDir + "/..").ToStdString(),         // one level up
                (exeDir).ToStdString()                  // exe directory itself
            };

            for (const auto& path : possiblePaths) {
                if (std::filesystem::exists(path + "/" + DEFAULT_MAP_NAME)) {
                    resourceDir_ = std::filesystem::path(path).string();
                    break;
                }
            }

            // If still not found, use current directory
            if (resourceDir_.empty()) {
                resourceDir_ = ".";
            }
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
