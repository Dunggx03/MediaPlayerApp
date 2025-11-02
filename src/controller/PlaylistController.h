#pragma once
#include "model/PlaylistManager.h"
#include "model/MediaFile.h"
#include "model/Playlist.h"

class PlaylistController {
public:
    PlaylistController(PlaylistManager* manager);

    bool createPlaylist(const std::string& name);
    bool deletePlaylist(const std::string& name);
    bool addTrackToPlaylist(MediaFile* file, Playlist* playlist);
    bool removeTrackFromPlaylist(MediaFile* file, Playlist* playlist);
private:
    PlaylistManager* playlistManager; // Non-owning pointer

};