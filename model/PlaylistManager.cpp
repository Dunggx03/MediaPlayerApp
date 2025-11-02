#include "../../include/model/PlaylistManager.h"
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

PlaylistManager::PlaylistManager() {
    // Tạo thư mục lưu playlist nếu chưa tồn tại
    if (!fs::exists(playlistDir)) {
        fs::create_directory(playlistDir);
        std::cout << "[PlaylistManager] Created directory: " << playlistDir << "\n";
    }
    // Tự động load toàn bộ playlist có sẵn
    loadAll();
}

// std::shared_ptr<Playlist> PlaylistManager::createPlaylist(const std::string& name) {
//     // Kiểm tra trùng tên playlist
//     for (const auto& pl : playlists) {
//         if (pl->getName() == name) {
//             std::cerr << "[PlaylistManager] Playlist already exists: " << name << "\n";
//             return nullptr;
//         }
//     }

//     // Tạo file lưu playlist
//     std::string path = playlistDir + "/" + name + ".txt";
//     std::ofstream file(path);
//     if (!file.is_open()) {
//         std::cerr << "[PlaylistManager] Cannot create playlist file: " << path << "\n";
//         return nullptr;
//     }
//     file.close();

//     // Tạo playlist mới trong bộ nhớ
//     auto newPlaylist = std::make_shared<Playlist>(name);
//     playlists.push_back(newPlaylist);

//     std::cout << "[PlaylistManager] Created playlist: " << name << "\n";
//     return newPlaylist;
// }
std::shared_ptr<Playlist> PlaylistManager::createPlaylist(const std::string& name) {
    // Nếu đã tồn tại trong bộ nhớ → trả về playlist sẵn có
    if (auto existing = findPlaylist(name)) {
        std::cout << "[PlaylistManager] Playlist already exists in memory: " << name << " (reloaded)\n";
        existing->loadFromFile(playlistDir + "/" + name + ".txt");
        return existing;
    }

    std::string path = playlistDir + "/" + name + ".txt";

    // Nếu file đã tồn tại → load thay vì tạo mới
    if (fs::exists(path)) {
        auto existing = std::make_shared<Playlist>(name);
        if (existing->loadFromFile(path)) {
            playlists.push_back(existing);
            std::cout << "[PlaylistManager] Loaded existing playlist from file: " << name << "\n";
            return existing;
        }
    }

    // Tạo playlist mới hoàn toàn
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "[PlaylistManager] Cannot create playlist file: " << path << "\n";
        return nullptr;
    }
    file.close();

    auto newPlaylist = std::make_shared<Playlist>(name);
    playlists.push_back(newPlaylist);

    std::cout << "[PlaylistManager] Created new playlist: " << name << "\n";
    return newPlaylist;
}


bool PlaylistManager::deletePlaylist(const std::string& name) {
    for (auto it = playlists.begin(); it != playlists.end(); ++it) {
        if ((*it)->getName() == name) {
            std::string path = playlistDir + "/" + name + ".txt";
            if (fs::exists(path)) {
                fs::remove(path);
                std::cout << "[PlaylistManager] Deleted file: " << path << "\n";
            }
            playlists.erase(it);
            std::cout << "[PlaylistManager] Deleted playlist: " << name << "\n";
            return true;
        }
    }
    std::cerr << "[PlaylistManager] Playlist not found: " << name << "\n";
    return false;
}

std::shared_ptr<Playlist> PlaylistManager::findPlaylist(const std::string& name) {
    for (auto& pl : playlists) {
        if (pl->getName() == name) {
            return pl;
        }
    }
    return nullptr;
}

const std::vector<std::shared_ptr<Playlist>>& PlaylistManager::getAllPlaylists() const {
    return playlists;
}

void PlaylistManager::loadAll() {
    if (!fs::exists(playlistDir)) {
        std::cerr << "[PlaylistManager] Directory not found: " << playlistDir << "\n";
        return;
    }

    for (const auto& entry : fs::directory_iterator(playlistDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            std::string name = entry.path().stem().string();
            if (findPlaylist(name)) continue;
            auto playlist = std::make_shared<Playlist>(name);

            // Mở file đọc từng dòng (mỗi dòng là path đến file media)
            std::ifstream file(entry.path());
            std::string line;
            while (std::getline(file, line)) {
                if (!line.empty() && fs::exists(line)) {
                    //playlist->addTrack(line, false);
                    playlist->addTrackFromFile(line);
                }
            }
            file.close();

            playlists.push_back(playlist);
            std::cout << "[PlaylistManager] Loaded playlist: " << name << "\n";
        }
    }
}
// void PlaylistManager::loadAll() {
//     if (!fs::exists(playlistDir)) return;

//     for (const auto& entry : fs::directory_iterator(playlistDir)) {
//         if (entry.is_regular_file() && entry.path().extension() == ".txt") {
//             std::string name = entry.path().stem().string();

//             // Bỏ qua nếu đã load rồi
//             if (findPlaylist(name)) continue;

//             auto playlist = std::make_shared<Playlist>(name);
//             std::ifstream file(entry.path());
//             std::string line;
//             while (std::getline(file, line)) {
//                 if (!line.empty()) playlist->addTrackFromFile(line);
//             }
//             playlists.push_back(playlist);
//             std::cout << "[PlaylistManager] Loaded playlist: " << name << "\n";
//         }
//     }
// }
