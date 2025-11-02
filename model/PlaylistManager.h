#ifndef PLAYLIST_MANAGER_H
#define PLAYLIST_MANAGER_H

#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include "Playlist.h"

// Quản lý danh sách playlist — hỗ trợ tự động lưu & tải từ file
class PlaylistManager {
private:
    std::vector<std::shared_ptr<Playlist>> playlists;
    std::string playlistDir = "Playlists";

public:
    PlaylistManager();

    std::shared_ptr<Playlist> createPlaylist(const std::string& name);
    bool deletePlaylist(const std::string& name);
    std::shared_ptr<Playlist> findPlaylist(const std::string& name);
    const std::vector<std::shared_ptr<Playlist>>& getAllPlaylists() const;

    void loadAll();  // đọc lại toàn bộ playlist từ thư mục
};

#endif
