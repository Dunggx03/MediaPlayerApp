#include "../../include/model/Playlist.h"
#include <filesystem>
#include <fstream>
#include <algorithm>

namespace fs = std::filesystem;

Playlist::Playlist(const std::string& name) : name(name) {}


// bool Playlist::addTrack(const std::string& path, bool saveToFile) {
//     if (std::find(trackPaths.begin(), trackPaths.end(), path) != trackPaths.end()) {
//         std::cout << "[Playlist] File already exists in playlist: " << path << "\n";
//         return false;
//     }
//     if (!fs::exists(path)) 
//         { std::cout << "[Playlist] Warning: File does not exist on disk: " << path << "\n"; } 
//     trackPaths.push_back(path);

//     if (saveToFile) { // chỉ ghi khi thật sự thêm mới
//         std::string playlistFile = "Playlists/" + name + ".txt";
//         std::ofstream out(playlistFile, std::ios::app);
//         if (!out.is_open()) {
//             std::cerr << "[Playlist] Failed to open file for append: " << playlistFile << "\n";
//             return false;
//         }
//         out << path << "\n";
//         out.close();
//     }

//     std::cout << "[Playlist] Added " << (saveToFile ? "and appended to file: " : "to memory: ") << path << "\n";
//     return true;
// }
bool Playlist::addTrack(const std::string& path) { 
    // Kiểm tra trùng trong bộ nhớ 
    if (std::find(trackPaths.begin(), trackPaths.end(), path) != trackPaths.end()) 
    { 
        std::cout << "[Playlist] File already exists in playlist: " << path << "\n"; return false; 
    } 
        // Tùy chọn: kiểm tra file tồn tại trên ổ đĩa 
        if (!fs::exists(path)) 
        { std::cout << "[Playlist] Warning: File does not exist on disk: " << path << "\n"; } 
        trackPaths.push_back(path); // Mở file ở chế độ append 
        std::string playlistFile = "Playlists/" + name + ".txt"; 
        std::ofstream out(playlistFile, std::ios::app); 
        if (!out.is_open())
        { 
            std::cerr << "[Playlist] Failed to open file for append: " << playlistFile << "\n"; 
            return false; 
        } 
            out << path << "\n"; 
            out.close(); 
            std::cout << "[Playlist] Added and appended to file: " << path << "\n"; 
            return true; 
}


bool Playlist::removeTrack(const std::string& fileName) {
    bool removed = false;
    std::string removedPath;

    // Tìm trong bộ nhớ
    for (auto it = trackPaths.begin(); it != trackPaths.end(); ++it) {
        if (fs::path(*it).filename() == fileName || *it == fileName) {
            removedPath = *it;
            trackPaths.erase(it);
            removed = true;
            break;
        }
    }

    if (!removed) {
        std::cout << "[Playlist] File not found in playlist: " << fileName << "\n";
        return false;
    }

    // Xóa đúng dòng trong file gốc
    std::string playlistFile = "Playlists/" + name + ".txt";
    std::ifstream in(playlistFile);
    if (!in.is_open()) {
        std::cerr << "[Playlist] Failed to open file for remove: " << playlistFile << "\n";
        return false;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line == removedPath || fs::path(line).filename() == fileName)
            continue;
        lines.push_back(line);
    }
    in.close();

    // Ghi lại file không có dòng bị xóa
    std::ofstream out(playlistFile, std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "[Playlist] Failed to rewrite playlist file: " << playlistFile << "\n";
        return false;
    }

    for (const auto& l : lines)
        out << l << "\n";

    out.close();

    std::cout << "[Playlist] Removed and updated file: " << removedPath << "\n";
    return true;
}


const std::vector<std::string>& Playlist::getTracks() const {
    return trackPaths;
}


std::vector<std::string> Playlist::getTrackNames() const {
    std::vector<std::string> names;
    names.reserve(trackPaths.size());
    for (const auto& path : trackPaths)
        names.push_back(fs::path(path).filename().string());
    return names;
}


const std::string& Playlist::getName() const {
    return name;
}


void Playlist::setName(const std::string& newName) {
    this->name = newName;
}


bool Playlist::saveToFile(const std::string& playlistFile) const {
    std::ofstream out(playlistFile, std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "[Playlist] Failed to save to file: " << playlistFile << "\n";
        return false;
    }

    for (const auto& path : trackPaths)
        out << path << "\n";

    return true;
}


bool Playlist::loadFromFile(const std::string& playlistFile) {
    std::ifstream in(playlistFile);
    if (!in.is_open())
        return false;

    trackPaths.clear();
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        trackPaths.push_back(line);
    }
    return true;
}
bool Playlist::addTrackFromFile(const std::string& path) {
    if (std::find(trackPaths.begin(), trackPaths.end(), path) != trackPaths.end())
        return false;
    trackPaths.push_back(path);
    return true;
}



