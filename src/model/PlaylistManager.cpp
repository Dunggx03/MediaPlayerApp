#include "model/PlaylistManager.h"
#include "model/MediaManager.h"
#include <algorithm> 
#include <iostream>
#include <fstream>
#include <filesystem>
#include "nlohmann/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

//Constructor 
PlaylistManager::PlaylistManager(MediaManager* manager) : mediaManager(manager) {
    if (mediaManager == nullptr) {
         std::cerr << "CRITICAL: PlaylistManager initialized with null MediaManager!" << std::endl;
    }
}

Playlist* PlaylistManager::createPlaylist(const std::string& name) {
    if (getPlaylistByName(name) != nullptr) {
        std::cerr << "PlaylistManager: Playlist with name '" << name << "' already exists." << std::endl;
        return nullptr;
    }

    auto newPlaylist = std::make_unique<Playlist>(name);
    
    Playlist* ptr = newPlaylist.get();
    
    this->playlists.push_back(std::move(newPlaylist));
    
    return ptr;
}

bool PlaylistManager::deletePlaylist(const std::string& name) {
    auto it = std::remove_if(playlists.begin(), playlists.end(), 
        [&name](const std::unique_ptr<Playlist>& p) {
            return p->getName() == name;
        });

    if (it != playlists.end()) {
        playlists.erase(it, playlists.end());
        return true;
    }

    return false;
}

Playlist* PlaylistManager::getPlaylistByName(const std::string& name) {
    auto it = std::find_if(playlists.begin(), playlists.end(),
        [&name](const std::unique_ptr<Playlist>& p) {
            return p->getName() == name;
        });

    if (it != playlists.end()) {
        return it->get(); // Return the raw pointer
    }

    return nullptr; // Not found
}

std::vector<Playlist*> PlaylistManager::getAllPlaylists() {
    std::vector<Playlist*> ptrs;
    ptrs.reserve(this->playlists.size());

    for (const auto& p : this->playlists) {
        ptrs.push_back(p.get());
    }
    
    return ptrs;
}


void PlaylistManager::saveToFile(const std::string& filename) {
    std::string path_to_save = filename;
    if (path_to_save.empty()) {
        path_to_save = savePath_; 
    }

    if (path_to_save.empty()) {
        std::cerr << "PlaylistManager Error: No save path specified. Cannot save." << std::endl;
        return;
    }
    
    json jsonData = json::array(); 

    for (const auto& playlistPtr : playlists) {
        if (!playlistPtr) continue; 

        json playlistObj; // JSON object for this playlist
        playlistObj["name"] = playlistPtr->getName();

        json tracksArray = json::array(); 
        for (const MediaFile* trackPtr : playlistPtr->getTracks()) {
            if (trackPtr) { 
                tracksArray.push_back(trackPtr->getFilePath()); // Store the full path
            }
        }
        playlistObj["tracks"] = tracksArray;

        jsonData.push_back(playlistObj); // Add playlist object to root array
    }

    try {
        fs::path p(path_to_save);
        if (p.has_parent_path()) {
            fs::create_directories(p.parent_path()); 
        }

        std::ofstream outFile(path_to_save);
        if (outFile.is_open()) {
            outFile << jsonData.dump(4);
            outFile.close();
            std::cout << "PlaylistManager: Playlists saved to " << path_to_save << std::endl;
        } else {
            std::cerr << "PlaylistManager Error: Could not open file for saving: " << path_to_save << std::endl;
        }
    } catch (fs::filesystem_error& e) {
         std::cerr << "PlaylistManager Error: Filesystem error: " << e.what() << std::endl;
    } catch (const json::exception& e) {
         std::cerr << "PlaylistManager Error: Failed to serialize JSON: " << e.what() << std::endl;
    }
}
void PlaylistManager::loadFromFile(const std::string& filename) {
    if (mediaManager == nullptr) {
        std::cerr << "PlaylistManager Error: Cannot load playlists - MediaManager is null." << std::endl;
        return;
    }
    savePath_ = filename;
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cout << "PlaylistManager Info: Playlist file not found or could not be opened: " << filename << ". Starting fresh." << std::endl;
        return;
    }

    try {
        json jsonData = json::parse(inFile);
        inFile.close(); // Close file after parsing

        playlists.clear();

        if (!jsonData.is_array()) {
            std::cerr << "PlaylistManager Error: Invalid playlist file format in " << filename << ". Expected a JSON array." << std::endl;
            return;
        }

        for (const auto& playlistObj : jsonData) {
            if (!playlistObj.is_object() || !playlistObj.contains("name") || !playlistObj["name"].is_string() || !playlistObj.contains("tracks") || !playlistObj["tracks"].is_array()) {
                 std::cerr << "PlaylistManager Warning: Skipping invalid or incomplete playlist object in " << filename << "." << std::endl;
                 continue;
            }

            std::string name = playlistObj["name"];
            Playlist* newPlaylist = createPlaylist(name);

            if (newPlaylist) {
                for (const auto& trackPathJson : playlistObj["tracks"]) {
                    if (trackPathJson.is_string()) 
                    {
                        std::string trackPath = trackPathJson;
                        MediaFile* file = nullptr;

                        if (mediaManager)
                            file = mediaManager->findFileByPath(trackPath);

                        if (!file && usbMediaManager)
                            file = usbMediaManager->findFileByPath(trackPath);

                        if (file)
                            newPlaylist->addTrack(file);
                        else
                            std::cerr << "PlaylistManager Warning: Track not found in any library: "
                                    << trackPath << std::endl;
                    } 
                    else 
                    {
                         std::cerr << "PlaylistManager Warning: Invalid track path type in playlist '" << name << "', skipping." << std::endl;
                    }
                }
            } 
            else 
            {
                 std::cerr << "PlaylistManager Warning: Could not create playlist '" << name << "' during load (maybe duplicate name?)." << std::endl;
            }
        }
        std::cout << "PlaylistManager: Playlists loaded successfully from " << filename << std::endl;

    } catch (json::parse_error& e) {
        std::cerr << "PlaylistManager Error: Failed to parse playlist JSON file " << filename << ": " << e.what() << std::endl;
        if(inFile.is_open()) inFile.close(); // Ensure file is closed on error
    } catch (const std::exception& e) {
         std::cerr << "PlaylistManager Error: An unexpected error occurred during playlist loading: " << e.what() << std::endl;
         if(inFile.is_open()) inFile.close();
    }
}

void PlaylistManager::autoSave() {
    std::cout << "[DEBUG] PlaylistManager: Auto-saving changes..." << std::endl;
    saveToFile(); 
}
void PlaylistManager::setUSBMediaManager(MediaManager* usbManager) {
    usbMediaManager = usbManager;
}

void PlaylistManager::removeTracksFromPathPrefix(const std::string& pathPrefix) {
    if (pathPrefix.empty()) return;

    std::cout << "[PlaylistManager]  Removing all tracks from USB path: " << pathPrefix << std::endl;

    for (auto& playlistPtr : playlists) {
        if (!playlistPtr) continue;
        std::vector<MediaFile*> cleanedTracks;

        for (auto* track : playlistPtr->getTracks()) {
            if (!track) continue;
            std::string path = track->getFilePath();

            if (path.rfind(pathPrefix, 0) != 0) {
                cleanedTracks.push_back(track);
            } else {
                std::cout << "  - Removed: " << path << std::endl;
            }
        }

        const_cast<std::vector<MediaFile*>&>(playlistPtr->getTracks()) = cleanedTracks;
    }
}

