#include "../../include/controller/MediaController.h"
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <fstream>

namespace fs = std::filesystem;

MediaController::MediaController()
    : localPath("Media/Local"),
      usbPath("Media/USB_Copy")
{
    // Tạo thư mục nếu chưa có
    fs::create_directories(localPath);
    fs::create_directories(usbPath);

    // Gắn callback từ MediaPlayer để xử lý event
    mediaPlayer.setCallback([this](const std::string& event, const MediaFile* file) {
        this->onPlayerEvent(event, file);
    });
}

MediaController::~MediaController() {
    mediaPlayer.stop();
}

// ===================================================
// 🔹 Nạp dữ liệu từ local hoặc USB
// ===================================================
bool MediaController::handleLoadSource(const std::string& sourceType) {
    if (sourceType == "local") {
        std::cout << "[MediaController] Loading media from local path...\n";
        mediaManager.loadFolder(localPath);
        if (viewCallback) viewCallback("reload", nullptr);
        return true;
    }
    else if (sourceType == "usb") {
        std::string mountPoint = "/media/usb"; // giả định mount sẵn
        if (!fs::exists(mountPoint)) {
            std::cerr << "[MediaController] USB not mounted at " << mountPoint << "\n";
            return false;
        }

        std::cout << "[MediaController] Copying from USB...\n";
        if (!copyFromUSB(mountPoint, usbPath)) {
            std::cerr << "[MediaController] Failed to copy USB contents.\n";
            return false;
        }

        mediaManager.loadFolder(usbPath);
        if (viewCallback) viewCallback("reload", nullptr);
        return true;
    }
    else {
        std::cerr << "[MediaController] Unknown source type: " << sourceType << "\n";
        return false;
    }
}

// ===================================================
// 🔹 Playback control
// ===================================================
void MediaController::playFile(const std::string& path) {
    const MediaFile* file = mediaManager.getMedia(path);
    if (!file) {
        std::cerr << "[MediaController] File not found: " << path << "\n";
        return;
    }

    std::vector<MediaFile*> list = mediaManager.getPage(0); // mặc định trang đầu
    mediaPlayer.setPlaylistAndPlay(list, 0);

    if (viewCallback) viewCallback("play", file);
}

void MediaController::playFromIndex(size_t index) {
    auto list = mediaManager.getPage(index / 25);
    mediaPlayer.setPlaylistAndPlay(list, index % 25);
}

void MediaController::pause() {
    mediaPlayer.pause();
}

void MediaController::resume() {
    mediaPlayer.resume();
}

void MediaController::stop() {
    mediaPlayer.stop();
}

void MediaController::next() {
    mediaPlayer.next();
}

void MediaController::previous() {
    mediaPlayer.previous();
}

void MediaController::setVolume(float value) {
    mediaPlayer.setVolume(value);
    if (viewCallback) viewCallback("volume", mediaPlayer.getCurrentTrack());
}

// ===================================================
// 🔹 View callback handler
// ===================================================
void MediaController::setViewCallback(ViewCallback cb) {
    viewCallback = std::move(cb);
}

// Khi MediaPlayer gửi event lên (observer pattern)
void MediaController::onPlayerEvent(const std::string& event, const MediaFile* file) {
    if (event == "finish") {
        std::cout << "[MediaController] Track finished -> auto next\n";
        mediaPlayer.next();
    }

    if (viewCallback) viewCallback(event, file);
}

// ===================================================
// 🔹 USB copy logic
// ===================================================
bool MediaController::copyFromUSB(const std::string& mountPoint, const std::string& destPath) {
    try {
        fs::create_directories(destPath);
        for (auto& entry : fs::recursive_directory_iterator(mountPoint)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".mp3" || ext == ".wav" || ext == ".flac" || ext == ".ogg") {
                    fs::copy_file(entry.path(), destPath + "/" + entry.path().filename().string(),
                                  fs::copy_options::overwrite_existing);
                }
            }
        }
        std::cout << "[MediaController] USB files copied successfully.\n";
        return true;
    } catch (std::exception& e) {
        std::cerr << "[MediaController] Copy failed: " << e.what() << "\n";
        return false;
    }
}

// ===================================================
// 🔹 Lấy dữ liệu phục vụ View
// ===================================================
std::vector<MediaFile*> MediaController::getCurrentPage(int pageNum) {
    return mediaManager.getPage(pageNum);
}

int MediaController::getTotalPages() const {
    // mediaManager.getTotalPages() is not const-qualified; cast away const to call it safely here
    return const_cast<decltype(mediaManager)&>(mediaManager).getTotalPages();
}

const MediaFile* MediaController::getCurrentTrack() const {
    return mediaPlayer.getCurrentTrack();
}

float MediaController::getVolume() const {
    return mediaPlayer.getVolume();
}
