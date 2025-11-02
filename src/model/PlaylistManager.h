#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Playlist.h"

#include "model/MediaManager.h"
class PlaylistManager {
private:
    std::vector<std::unique_ptr<Playlist>> playlists;
    MediaManager* mediaManager;
    MediaManager* usbMediaManager;
    std::string savePath_;
public:
    explicit PlaylistManager(MediaManager* manager);
    Playlist* createPlaylist(const std::string& name);
    Playlist* getPlaylistByName(const std::string& name);
    bool deletePlaylist(const std::string& name);

    std::vector<Playlist*> getAllPlaylists();
    void saveToFile(const std::string& filename = "");
    void loadFromFile(const std::string& filename);
    void autoSave();
    void setUSBMediaManager(MediaManager* usbManager);

    void removeTracksFromPathPrefix(const std::string& pathPrefix);


};