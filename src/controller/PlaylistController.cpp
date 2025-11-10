#include "controller/PlaylistController.h"
#include <iostream> 

// Constructor
PlaylistController::PlaylistController(PlaylistManager* manager)
    : playlistManager(manager) 
{
    if (this->playlistManager == nullptr) {
        std::cerr << "CRITICAL: PlaylistController initialized with a null PlaylistManager!" << std::endl;
    }
}

// createPlaylist 
bool PlaylistController::createPlaylist(const std::string& name) {
    if (name.empty()) {
        std::cerr << "Controller Error: Playlist name cannot be empty." << std::endl;
        return false;
    }
    Playlist* p = playlistManager->createPlaylist(name);
    //return (p != nullptr);
    if (p) {
        playlistManager->autoSave(); 
        return true;
    }
    return false;
}

// deletePlaylist
bool PlaylistController::deletePlaylist(const std::string& name) {
    if (name.empty()) {
        return false;
    }
    
    bool success = playlistManager->deletePlaylist(name);
    if (success) {
        playlistManager->autoSave(); 
    }
    return success;
}

// addTrackToPlaylist 
bool PlaylistController::addTrackToPlaylist(MediaFile* file, Playlist* playlist) {
    if (file == nullptr || playlist == nullptr) {
        std::cerr << "Controller Error: Cannot add null track or to null playlist." << std::endl;
        return false;
    }
    
    playlist->addTrack(file); 
    
    //add logic to save the playlist to file
    playlistManager->autoSave();
    
    return true;
}

bool PlaylistController::removeTrackFromPlaylist(MediaFile* file, Playlist* playlist) {
    if (file == nullptr || playlist == nullptr) {
        std::cerr << "Controller Error: Cannot remove null track or from null playlist." << std::endl;
        return false;
    }
    
    bool success = playlist->removeTrack(file);
    if (success) {
        playlistManager->autoSave();
    }
    return success;
}

