#include "controller/AppController.h"

#include "utils/TagLibWrapper.h"
#include "utils/SDLWrapper.h"
#include "utils/DeviceConnector.h"
#include "utils/USBUtils.h"
#include "utils/FileUtils.h"
#include "model/MediaManager.h"
#include "model/PlaylistManager.h"
#include "model/MediaPlayer.h"

#include "controller/MediaController.h"
#include "controller/PlaylistController.h"
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <unistd.h> 

namespace fs = std::filesystem;

static fs::path getUserMusicRoot() {
    const char* home = getenv("HOME");
    if (!home) home = getlogin();
    return fs::path(home) / "Music" / "MediaPlayer";
}

AppController::AppController() {}
AppController::~AppController() {}

bool AppController::init() {
    tagLibWrapper = std::make_unique<TagLibWrapper>();
    sdlWrapper = std::make_unique<SDLWrapper>();
    deviceConnector = std::make_unique<DeviceConnector>();
    usbUtils = std::make_unique<USBUtils>();

    if (!sdlWrapper->init()) {
        std::cerr << "CRITICAL: Failed to initialize SDLWrapper!" << std::endl;
        return false;
    }

    mediaPlayer = std::make_unique<MediaPlayer>(sdlWrapper.get());
    mediaManager = std::make_unique<MediaManager>(tagLibWrapper.get());
    usbMediaManager = std::make_unique<MediaManager>(tagLibWrapper.get());

    playlistManager = std::make_unique<PlaylistManager>(mediaManager.get());
    playlistManager->setUSBMediaManager(usbMediaManager.get());

    mediaController = std::make_unique<MediaController>(
        mediaManager.get(), mediaPlayer.get(),
        tagLibWrapper.get(), deviceConnector.get()
    );

    usbmediaController = std::make_unique<MediaController>(
        usbMediaManager.get(), mediaPlayer.get(),
        tagLibWrapper.get(), deviceConnector.get()
    );

    playlistController = std::make_unique<PlaylistController>(playlistManager.get());

    // Callback auto next track
    mediaPlayer->setOnTrackFinishedCallback([this]() {
        if (!this->mediaPlayer) return;
        MediaFile* current = this->mediaPlayer->getCurrentTrack();
        if (!current) return;

        if (!this->currentUSBPath.empty() &&
            current->getFilePath().rfind(this->currentUSBPath, 0) == 0) {
            if (this->usbmediaController)
                this->usbmediaController->nextTrack();
        } else {
            if (this->mediaController)
                this->mediaController->nextTrack();
        }
    });

    return true;
}

bool AppController::loadUSBLibrary() {
    if (!usbUtils || !usbMediaManager) return false;

    currentUSBPath = usbUtils->detectUSBMount();
    if (currentUSBPath.empty()) {
        std::cerr << "[AppController]  No USB detected.\n";
        return false;
    }

    std::cout << "[AppController] Loading media from: " << currentUSBPath << std::endl;
    usbMediaManager->loadFromDirectory(currentUSBPath);
    usbmediaController = std::make_unique<MediaController>(
        usbMediaManager.get(), mediaPlayer.get(),
        tagLibWrapper.get(), deviceConnector.get()
    );

    if (playlistManager)
        playlistManager->setUSBMediaManager(usbMediaManager.get());

    fs::path root = getUserMusicRoot();
    fs::path playlistPath = root / "playlist" / "playlists.json";
    if (playlistManager) {
        std::cout << "[AppController]  Reloading playlists after USB mount...\n";
        playlistManager->loadFromFile(playlistPath.string());
    }
    return true;
}

bool AppController::ejectUSB() {
    if (!usbUtils || currentUSBPath.empty()) return false;

    if (mediaPlayer) {
        MediaFile* current = mediaPlayer->getCurrentTrack();
        if (current && current->getFilePath().rfind(currentUSBPath, 0) == 0)
            mediaPlayer->stop();
    }

    bool ok = usbUtils->unmountUSB(currentUSBPath);

    if (ok && usbMediaManager ) {
        usbMediaManager->clearLibrary();
        std::cout << "[AppController]  USB unmounted safely.\n";

        fs::path root = getUserMusicRoot();
        fs::path playlistPath = root / "playlist" / "playlists.json";
        if (playlistManager) {
            std::cout << "[AppController] Reloading playlists after USB eject...\n";
            playlistManager->loadFromFile(playlistPath.string());
        }
    } else {
        std::cerr << "[AppController] Failed to unmount USB.\n";
    }

    currentUSBPath.clear();
    return ok;
}

// Getters
MediaManager* AppController::getMediaManager() const { return mediaManager.get(); }
PlaylistManager* AppController::getPlaylistManager() const { return playlistManager.get(); }
MediaPlayer* AppController::getMediaPlayer() const { return mediaPlayer.get(); }
MediaController* AppController::getMediaController() const { return mediaController.get(); }
PlaylistController* AppController::getPlaylistController() const { return playlistController.get(); }
MediaManager* AppController::getUSBMediaManager() const { return usbMediaManager.get(); }
MediaController* AppController::getusbmediaController() const { return usbmediaController.get(); }
