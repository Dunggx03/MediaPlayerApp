#include "../../include/model/MediaManager.h"
#include <string>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

bool endsWith(const std::string& str, const std::string& suffix) {
    if (str.size() < suffix.size()) return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

void MediaManager::loadFolder(const string& folderPath){

    mediaLibrary.clear();
    if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) {
        std::cerr << "[MediaManager] Invalid folder path: " << folderPath << "\n";
        return;
    }
    for (const auto& entry : fs::recursive_directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string path = entry.path().string();
            if (endsWith(path, ".mp3") ||
                endsWith(path, ".wav") ||
                endsWith(path, ".flac") ||
                endsWith(path, ".aac") ||
                endsWith(path, ".ogg") ||
                endsWith(path, ".mp4") ||
                endsWith(path, ".mkv")) 
            {
                mediaLibrary.emplace_back(path);
            }
        }
    }
}

std::vector<MediaFile*> MediaManager::getPage(int pageNum) {
    std::vector<MediaFile*> result;

    int totalPages = getTotalPages();
    if (pageNum <= 0 || pageNum > totalPages) return result;

    size_t start = (pageNum - 1)* 25;
    size_t remaining = mediaLibrary.size() - start;
    size_t count = std::min<size_t>(25, remaining);

    result.reserve(count);

    for (size_t i = 0; i < count; i++) {
        result.push_back(&mediaLibrary[start + i]);
    }

    return result;
}

int MediaManager::getTotalPages() {
    return (mediaLibrary.size() + 24) / 25;
}

const MediaFile* MediaManager::getMedia(const std::string& path) const {
    for (const auto& file : mediaLibrary) {
        if (file.getPath() == path)
            return &file;
    }
    return nullptr; 
}

