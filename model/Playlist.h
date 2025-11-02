#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

class Playlist {
private:
    std::string name;
    std::vector<std::string> trackPaths; // lưu đường dẫn file nhạc

public:
    explicit Playlist(const std::string& name);

    bool addTrack(const std::string& path);
    //bool addTrack(const std::string& path, bool saveToFile = true ) ;
    bool removeTrack(const std::string& fileName);

    const std::vector<std::string>& getTracks() const;
    std::vector<std::string> getTrackNames() const;

    const std::string& getName() const;
    void setName(const std::string& newName);

    bool saveToFile(const std::string& playlistFile) const;
    bool loadFromFile(const std::string& playlistFile);

    bool addTrackFromFile(const std::string& path);
    //void loadTrack(const MediaFile& track);

};

#endif // PLAYLIST_H
