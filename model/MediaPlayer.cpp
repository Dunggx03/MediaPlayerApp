#include "../../include/model/MediaPlayer.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>

namespace fs = std::filesystem;

MediaPlayer::MediaPlayer() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "[MediaPlayer] SDL init failed: " << SDL_GetError() << "\n";
    }

    int flags = MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_FLAC;
    if (Mix_Init(flags) != flags) {
        std::cerr << "[MediaPlayer] SDL_mixer init failed: " << Mix_GetError() << "\n";
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "[MediaPlayer] Mix_OpenAudio failed: " << Mix_GetError() << "\n";
    }

    Mix_VolumeMusic(static_cast<int>(volume.load() * MIX_MAX_VOLUME));
}

MediaPlayer::~MediaPlayer() {
    stop();
    Mix_CloseAudio();
    Mix_Quit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void MediaPlayer::setCallback(Callback cb) {
    callback = std::move(cb);
}

void MediaPlayer::setPlaylistAndPlay(const std::vector<MediaFile*>& newList, size_t index) {
    std::lock_guard<std::mutex> lock(mtx);
    stop();
    playlist = newList;
    play(index);
}

void MediaPlayer::play(size_t index) {
    std::lock_guard<std::mutex> lock(mtx);
    if (index >= playlist.size()) return;

    // Dừng bài hiện tại nếu có
    stopRequested = true;
    if (playThread.joinable()) playThread.join();
    cleanup();

    currentIndex = index;
    stopRequested = false;
    isPlaying = true;

    const MediaFile* track = playlist[currentIndex];
    if (callback) callback("play", track);

    playThread = std::thread(&MediaPlayer::playbackThread, this, playlist[currentIndex]);
}

void MediaPlayer::pause() {
    if (isPlaying && Mix_PlayingMusic()) {
        Mix_PauseMusic();
        isPlaying = false;
        if (callback) callback("pause", getCurrentTrack());
    }
}

void MediaPlayer::resume() {
    if (!isPlaying && Mix_PausedMusic()) {
        Mix_ResumeMusic();
        isPlaying = true;
        if (callback) callback("resume", getCurrentTrack());
    }
}

void MediaPlayer::stop() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        stopRequested = true;
    }

    if (playThread.joinable()) {
        playThread.join();
    }

    cleanup();
    if (callback) callback("stop", getCurrentTrack());
}

void MediaPlayer::next() {
    std::lock_guard<std::mutex> lock(mtx);
    if (playlist.empty()) return;
    size_t nextIndex = (currentIndex + 1) % playlist.size();
    if (callback) callback("next", playlist[nextIndex]);
    play(nextIndex);
}

void MediaPlayer::previous() {
    std::lock_guard<std::mutex> lock(mtx);
    if (playlist.empty()) return;
    size_t prevIndex = (currentIndex == 0) ? playlist.size() - 1 : currentIndex - 1;
    if (callback) callback("previous", playlist[prevIndex]);
    play(prevIndex);
}

void MediaPlayer::setVolume(float value) {
    float clamped = std::clamp(value, 0.0f, 1.0f);
    volume.store(clamped);
    Mix_VolumeMusic(static_cast<int>(clamped * MIX_MAX_VOLUME));
    if (callback) callback("volume", getCurrentTrack());
}

float MediaPlayer::getVolume() const {
    return volume.load();
}

const MediaFile* MediaPlayer::getCurrentTrack() const {
    if (playlist.empty() || currentIndex >= playlist.size()) return nullptr;
    return playlist[currentIndex];
}

float MediaPlayer::getCurrentTime() const {
    return currentTime.load();
}

// ================== PRIVATE ================== //

void MediaPlayer::playbackThread(MediaFile* file) {
    if (!file) return;

    std::string path = file->getPath();
    if (!fs::exists(path)) {
        std::cerr << "[MediaPlayer] File not found: " << path << "\n";
        if (callback) callback("error", file);
        return;
    }

    currentMusic = Mix_LoadMUS(path.c_str());
    if (!currentMusic) {
        std::cerr << "[MediaPlayer] Mix_LoadMUS error: " << Mix_GetError() << "\n";
        if (callback) callback("error", file);
        return;
    }

    if (Mix_PlayMusic(currentMusic, 1) == -1) {
        std::cerr << "[MediaPlayer] Mix_PlayMusic error: " << Mix_GetError() << "\n";
        if (callback) callback("error", file);
        return;
    }

    currentTime = 0.0f;
    auto startTime = std::chrono::steady_clock::now();

    while (Mix_PlayingMusic() && !stopRequested) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - startTime)
                           .count();
        currentTime = elapsed / 1000.0f;
    }

    cleanup();

    if (!stopRequested) {
        if (callback) callback("finish", file);
        autoNextWhenFinish();
    }
}

void MediaPlayer::autoNextWhenFinish() {
    if (playlist.empty()) return;

    size_t nextIndex = (currentIndex + 1) % playlist.size();
    if (callback) callback("next", playlist[nextIndex]);
    play(nextIndex);
}

void MediaPlayer::cleanup() {
    if (currentMusic) {
        Mix_FreeMusic(currentMusic);
        currentMusic = nullptr;
    }
    isPlaying = false;
}
