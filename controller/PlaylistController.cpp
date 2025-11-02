#include "../../include/controller/PlaylistController.h"
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

PlaylistController::PlaylistController(MediaManager* mediaMgr, MediaPlayer* player)
    : mediaManager(mediaMgr), mediaPlayer(player)
{
    std::cout << "[PlaylistController] Initialized.\n";
}

void PlaylistController::setViewCallback(ViewCallback cb) {
    viewCallback = std::move(cb);
}

void PlaylistController::notifyView(const std::string& event, const std::string& name) {
    if (viewCallback) viewCallback(event, name);
}

// ==========================================================
// 🎵 Playlist CRUD
// ==========================================================

bool PlaylistController::createPlaylist(const std::string& name) {
    auto pl = playlistManager.createPlaylist(name);
    if (!pl) {
        std::cerr << "[PlaylistController] Failed to create playlist: " << name << "\n";
        return false;
    }
    std::cout << "[PlaylistController] Created playlist: " << name << "\n";
    notifyView("playlist_created", name);
    return true;
}

bool PlaylistController::deletePlaylist(const std::string& name) {
    if (!playlistManager.deletePlaylist(name)) {
        std::cerr << "[PlaylistController] Delete failed: " << name << "\n";
        return false;
    }
    std::cout << "[PlaylistController] Deleted playlist: " << name << "\n";
    notifyView("playlist_deleted", name);
    return true;
}

// ==========================================================
// 🎶 Track management
// ==========================================================

bool PlaylistController::addToPlaylist(const std::string& playlistName, const std::string& mediaPath) {
    auto pl = playlistManager.findPlaylist(playlistName);
    if (!pl) {
        std::cerr << "[PlaylistController] Playlist not found: " << playlistName << "\n";
        return false;
    }

    if (!fs::exists(mediaPath)) {
        std::cerr << "[PlaylistController] Media file not found: " << mediaPath << "\n";
        return false;
    }

    if (!pl->addTrack(mediaPath)) {
        std::cerr << "[PlaylistController] Failed to add track: " << mediaPath << "\n";
        return false;
    }

    std::cout << "[PlaylistController] Added track to " << playlistName << ": " << mediaPath << "\n";
    notifyView("track_added", playlistName);
    return true;
}

bool PlaylistController::removeFromPlaylist(const std::string& playlistName, const std::string& trackName) {
    auto pl = playlistManager.findPlaylist(playlistName);
    if (!pl) {
        std::cerr << "[PlaylistController] Playlist not found: " << playlistName << "\n";
        return false;
    }

    if (!pl->removeTrack(trackName)) {
        std::cerr << "[PlaylistController] Failed to remove track: " << trackName << "\n";
        return false;
    }

    std::cout << "[PlaylistController] Removed " << trackName << " from " << playlistName << "\n";
    notifyView("track_removed", playlistName);
    return true;
}

// ==========================================================
// 📜 View playlist & playback
// ==========================================================

std::vector<std::string> PlaylistController::viewPlaylist(const std::string& name) {
    auto pl = playlistManager.findPlaylist(name);
    if (!pl) {
        std::cerr << "[PlaylistController] Playlist not found: " << name << "\n";
        return {};
    }

    std::cout << "[PlaylistController] Viewing playlist: " << name << "\n";
    auto tracks = pl->getTrackNames();
    for (size_t i = 0; i < tracks.size(); ++i) {
        std::cout << "  " << i + 1 << ". " << tracks[i] << "\n";
    }

    notifyView("playlist_view", name);
    return tracks;
}

void PlaylistController::playPlaylist(const std::string& name, size_t startIndex) {
    auto pl = playlistManager.findPlaylist(name);
    if (!pl) {
        std::cerr << "[PlaylistController] Playlist not found: " << name << "\n";
        return;
    }

    auto paths = pl->getTracks();
    std::vector<MediaFile*> mediaList;
    mediaList.reserve(paths.size());

    for (auto& path : paths) {
        // lấy MediaFile từ MediaManager để tận dụng metadata
        const MediaFile* mf = mediaManager ? mediaManager->getMedia(path) : nullptr;
        if (mf) {
            mediaList.push_back(const_cast<MediaFile*>(mf));
        } else {
            mediaList.push_back(new MediaFile(path)); // fallback
        }
    }

    if (mediaList.empty()) {
        std::cerr << "[PlaylistController] Playlist empty: " << name << "\n";
        return;
    }

    mediaPlayer->setPlaylistAndPlay(mediaList, startIndex);
    notifyView("playlist_play", name);
}

// ==========================================================
// 📋 Utility
// ==========================================================

std::vector<std::string> PlaylistController::getAllPlaylists() const {
    std::vector<std::string> names;
    for (auto& p : playlistManager.getAllPlaylists())
        names.push_back(p->getName());
    return names;
}
