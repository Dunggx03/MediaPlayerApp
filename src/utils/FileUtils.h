#pragma once
#include <string>
#include <vector>
#include <filesystem>
namespace fs = std::filesystem;
namespace FileUtils {
    std::vector<std::string> getMediaFilesRecursive(const std::string& rootPath);
    bool isMediaFile(const std::string& filePath);
    fs::path getProjectRootPath();
}