#ifndef MEDIA_MANAGER_H
#define MEDIA_MANAGER_H

#include <vector>
#include <span>
#include "MediaFile.h"

class MediaManager {
private:
    std::vector<MediaFile> mediaLibrary;
public:

    void loadFolder(const std::string& path);

    std::vector<MediaFile*> getPage(int pageNum) ;

    int getTotalPages();

    const MediaFile* getMedia(const std::string& path) const;

};

#endif