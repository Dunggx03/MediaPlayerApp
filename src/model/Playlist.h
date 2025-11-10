#pragma once
#include <string>
#include <vector>
#include "MediaFile.h"

class Playlist {
public:
    Playlist(const std::string& name);

    std::string getName() const;
    void setName(const std::string& newName);
void addTrack(MediaFile* file);
    
    bool removeTrack(MediaFile* file);

    const std::vector<MediaFile*>& getTracks() const;

private:
    std::string name;
    std::vector<MediaFile*> tracks;
};