#include "model/Playlist.h"
#include <algorithm> 

Playlist::Playlist(const std::string& name) : name(name) {}

std::string Playlist::getName() const {
    return this->name;
}

void Playlist::setName(const std::string& newName) {
    this->name = newName;
}

void Playlist::addTrack(MediaFile* file) {
    if (file == nullptr) {
        return;
    }
    auto it = std::find(tracks.begin(), tracks.end(), file);
    if (it == tracks.end()) { // Add only if not already present
        tracks.push_back(file);
    }
}

bool Playlist::removeTrack(MediaFile* file) {
    if (file == nullptr) {
        return false;
    }
    auto it = std::remove(tracks.begin(), tracks.end(), file);

    if (it != tracks.end()) {
        tracks.erase(it, tracks.end()); // Erase the shifted element(s)
        return true;
    }
    return false; // Element not found
}

const std::vector<MediaFile*>& Playlist::getTracks() const {
    return this->tracks;
}

