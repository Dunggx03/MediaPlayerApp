#include "../../include/controller/ControllerManager.h"
#include <iostream>
#include <thread>
#include <chrono>

ControllerManager::ControllerManager()
    : currentMenu(ActiveMenu::MEDIA_MENU)
{
    std::cout << "[ControllerManager] Initializing controllers...\n";

    // Tạo MediaManager & MediaPlayer chia sẻ chung
    mediaManager = std::make_unique<MediaManager>();
    mediaPlayer  = std::make_unique<MediaPlayer>();

    // Tạo Controller con
    mediaController    = std::make_unique<MediaController>();
    playlistController = std::make_unique<PlaylistController>(mediaManager.get(), mediaPlayer.get());

    // Kết nối callback giữa MediaPlayer -> MediaController -> ControllerManager -> View
    mediaController->setViewCallback([this](const std::string& event, const MediaFile* file) {
        std::string info = file ? file->getName() : "";
        relayEventToView(event, info);
    });

    playlistController->setViewCallback([this](const std::string& event, const std::string& name) {
        relayEventToView(event, name);
    });

    std::cout << "[ControllerManager] Controllers ready.\n";
}

ControllerManager::~ControllerManager() {
    std::cout << "[ControllerManager] Cleaning up controllers...\n";
}

// ======================================================
// 🔹 Bắt đầu chương trình
// ======================================================
void ControllerManager::start() {
    std::cout << "\n===== MEDIA PLAYER CLI STARTED =====\n";
    relayEventToView("app_started", "");

    // Mặc định load local source khi khởi động
    mediaController->handleLoadSource("local");
    relayEventToView("media_loaded", "local");

    // Gửi thông báo cho View
    relayEventToView("menu_active", "MEDIA_MENU");
}

// ======================================================
// 🔹 Chuyển menu
// ======================================================
void ControllerManager::switchToMediaMenu() {
    currentMenu = ActiveMenu::MEDIA_MENU;
    relayEventToView("menu_switched", "MEDIA_MENU");
    std::cout << "[ControllerManager] Switched to Media Menu.\n";
}

void ControllerManager::switchToPlaylistMenu() {
    currentMenu = ActiveMenu::PLAYLIST_MENU;
    relayEventToView("menu_switched", "PLAYLIST_MENU");
    std::cout << "[ControllerManager] Switched to Playlist Menu.\n";
}

// ======================================================
// 🔹 Gán callback lên View
// ======================================================
void ControllerManager::setViewCallback(ViewCallback cb) {
    viewCallback = std::move(cb);
}

void ControllerManager::relayEventToView(const std::string& event, const std::string& info) {
    if (viewCallback) {
        viewCallback(event, info);
    }
}

// ======================================================
// 🔹 Gửi event từ View đến Controller tương ứng
// ======================================================
void ControllerManager::handleUserAction(const std::string& action, const std::string& argument) {
    std::cout << "[ControllerManager] Action received: " << action
              << " (" << argument << ")\n";

    // --- Menu Media ---
    if (currentMenu == ActiveMenu::MEDIA_MENU) {
        if (action == "load_local") {
            mediaController->handleLoadSource("local");
        }
        else if (action == "load_usb") {
            mediaController->handleLoadSource("usb");
        }
        else if (action == "play") {
            mediaController->playFile(argument);
        }
        else if (action == "pause") {
            mediaController->pause();
        }
        else if (action == "resume") {
            mediaController->resume();
        }
        else if (action == "next") {
            mediaController->next();
        }
        else if (action == "previous") {
            mediaController->previous();
        }
        else if (action == "volume") {
            try {
                float v = std::stof(argument);
                mediaController->setVolume(v);
            } catch (...) { std::cerr << "[ControllerManager] Invalid volume value.\n"; }
        }
        else if (action == "switch_playlist") {
            switchToPlaylistMenu();
        }
    }

    // --- Menu Playlist ---
    else if (currentMenu == ActiveMenu::PLAYLIST_MENU) {
        if (action == "create_playlist") {
            playlistController->createPlaylist(argument);
        }
        else if (action == "delete_playlist") {
            playlistController->deletePlaylist(argument);
        }
        else if (action == "view_playlist") {
            playlistController->viewPlaylist(argument);
        }
        else if (action == "add_track") {
            size_t sep = argument.find('|');
            if (sep != std::string::npos) {
                std::string playlistName = argument.substr(0, sep);
                std::string trackPath = argument.substr(sep + 1);
                playlistController->addToPlaylist(playlistName, trackPath);
            }
        }
        else if (action == "remove_track") {
            size_t sep = argument.find('|');
            if (sep != std::string::npos) {
                std::string playlistName = argument.substr(0, sep);
                std::string trackName = argument.substr(sep + 1);
                playlistController->removeFromPlaylist(playlistName, trackName);
            }
        }
        else if (action == "play_playlist") {
            playlistController->playPlaylist(argument, 0);
        }
        else if (action == "switch_media") {
            switchToMediaMenu();
        }
    }
}

// ======================================================
// 🔹 Truy xuất controller con
// ======================================================
MediaController& ControllerManager::getMediaController() {
    return *mediaController;
}

PlaylistController& ControllerManager::getPlaylistController() {
    return *playlistController;
}
