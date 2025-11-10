#include "model/MediaManager.h"
#include "utils/TagLibWrapper.h" 
#include "utils/FileUtils.h"     
#include <iostream>
#include <cmath> 
#include <algorithm>
#include <filesystem>
#include <unordered_map>

MediaManager::MediaManager(TagLibWrapper* tagUtil)
    : tagUtil(tagUtil) 
{}

void MediaManager::loadFromDirectory(const std::string& path) {
    std::cout << "MediaManager: Loading from directory: " << path << std::endl;
    this->clearLibrary();

    std::vector<std::string> files = FileUtils::getMediaFilesRecursive(path);
    
    std::cout << "MediaManager: Found " << files.size() << " media files." << std::endl;

    for (const auto& file : files) {
        std::unique_ptr<Metadata> metadata = this->tagUtil->readTags(file);
        if (metadata) {
            this->library.push_back(std::make_unique<MediaFile>(file, std::move(metadata)));
        } else {
            std::cerr << "MediaManager: Skipping file (could not read metadata): " << file << std::endl;
        }
    }
    std::cout << "MediaManager: Load complete. Library size: " << this->library.size() << std::endl;
}

void MediaManager::clearLibrary() {
    this->library.clear();
}

std::vector<MediaFile*> MediaManager::getPage(int pageNumber, int pageSize) {
    std::vector<MediaFile*> page;
    
    if (pageNumber < 1) {
        pageNumber = 1;
    }

    int start = (pageNumber - 1) * pageSize;
    
    if (start < 0 || static_cast<size_t>(start) >= this->library.size()) {
        return page; // Return empty vector
    }

    int end = start + pageSize;
    
    if (static_cast<size_t>(end) > this->library.size()) {
        end = this->library.size();
    }

    for (int i = start; i < end; ++i) {
        page.push_back(this->library[i].get()); // Add non-owning pointer
    }
    
    return page;
}

int MediaManager::getTotalPages(int pageSize) const {
    if (pageSize <= 0) return 0;
    if (this->library.empty()) return 1; // Always at least one page, even if empty
    
    return static_cast<int>(std::ceil(static_cast<double>(this->library.size()) / pageSize));
}

int MediaManager::getTotalFileCount() const {
    return this->library.size();
}

MediaFile* MediaManager::findFileByPath(const std::string& filePath) const {
    auto it = std::find_if(library.begin(), library.end(),
        [&filePath](const std::unique_ptr<MediaFile>& filePtr) {
            return filePtr->getFilePath() == filePath;
        });

    if (it != library.end()) {
        return it->get(); // Return raw pointer
    }

    return nullptr; // Not found
}
