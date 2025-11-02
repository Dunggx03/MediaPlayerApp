#pragma once
#include <vector>
#include <string>
#include <memory>
#include "MediaFile.h"

class TagLibWrapper;

class MediaManager {
private:
    std::vector<std::unique_ptr<MediaFile>> library;
    TagLibWrapper* tagUtil;
public:
    MediaManager(TagLibWrapper* tagUtil);

    void loadFromDirectory(const std::string& path);
    void clearLibrary();

    std::vector<MediaFile*> getPage(int pageNumber, int pageSize = 25);
    int getTotalPages(int pageSize = 25) const;
    int getTotalFileCount() const;
    MediaFile* findFileByPath(const std::string& filePath) const;

};