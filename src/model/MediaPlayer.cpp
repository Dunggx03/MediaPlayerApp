#include "model/MediaPlayer.h"
#include <iostream>

MediaPlayer::MediaPlayer(SDLWrapper* sdlWrapper)
    : sdlWrapper(sdlWrapper), 
      currentTrack(nullptr), 
      currentState(PlayerState::STOPPED), 
      currentVolume(100), // Default volume
      onTrackFinishedCallback_(nullptr),
      isStoppingManually_(false),
      activePlaylist_(nullptr)
{
    if (sdlWrapper == nullptr) {
        std::cerr << "CRITICAL: MediaPlayer started with null SDLWrapper!" << std::endl;
        return;
    }

    sdlWrapper->setTrackFinishedCallback([this]() {
        this->onTrackFinished();
    });
}

void MediaPlayer::play(MediaFile* file, Playlist* context) {
    if (file == nullptr) return;

    isStoppingManually_ = true;

    if (sdlWrapper->playAudio(file->getFilePath())) {
        currentTrack = file;
        currentState = PlayerState::PLAYING;
        activePlaylist_ = context;
    } else {
        currentTrack = nullptr;
        currentState = PlayerState::STOPPED;
        activePlaylist_ = nullptr;
    }
    isStoppingManually_ = false;
}

void MediaPlayer::pause() {
    if (currentState == PlayerState::PLAYING) {
        sdlWrapper->pauseAudio(); // Pause
        currentState = PlayerState::PAUSED;
    } else if (currentState == PlayerState::PAUSED) {
        sdlWrapper->pauseAudio(); // Resume
        currentState = PlayerState::PLAYING;
    }
}

void MediaPlayer::stop() {
    isStoppingManually_ = true;
    sdlWrapper->stopAudio();
    currentState = PlayerState::STOPPED;
    currentTrack = nullptr;
    activePlaylist_ = nullptr;
    isStoppingManually_ = false;
}

void MediaPlayer::setVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    currentVolume = volume;
    
    // SDL_mixer volume is 0-128
    int sdlVolume = (currentVolume * MIX_MAX_VOLUME) / 100;
    Mix_VolumeMusic(sdlVolume);
}

int MediaPlayer::getVolume() const {
    return currentVolume;
}

PlayerState MediaPlayer::getState() const {
    return currentState;
}

int MediaPlayer::getCurrentTime() const {
    if (currentState == PlayerState::STOPPED) {
        return 0;
    }
    return sdlWrapper->getCurrentTime();
}

int MediaPlayer::getTotalTime() const {
    if (currentTrack) {
        Metadata* meta = currentTrack->getMetadata();
        if (meta) {
            return meta->durationInSeconds;
        }
    }
    return 0;
}

MediaFile* MediaPlayer::getCurrentTrack() const {
    return currentTrack;
}

void MediaPlayer::onTrackFinished() {
    std::cout << "MediaPlayer: Track finished." << std::endl;
    if (isStoppingManually_) {
        std::cout << "MediaPlayer: Manual stop detected, ignoring auto-next." << std::endl;
        return;
    }

    MediaFile* finishedTrack = currentTrack;
    currentState = PlayerState::STOPPED;

    //Auto-next 
    if (onTrackFinishedCallback_) {
        onTrackFinishedCallback_();
    }
    if (currentTrack == finishedTrack) {
        std::cout << "MediaPlayer: Auto-next did not start new track. Setting track to null." << std::endl;
        currentTrack = nullptr;
    } else {
        std::cout << "MediaPlayer: Auto-next successfully started a new track." << std::endl;
    }
}

void MediaPlayer::setOnTrackFinishedCallback(std::function<void()> callback) {
    onTrackFinishedCallback_ = callback;
}

Playlist* MediaPlayer::getActivePlaylist() const {
    return activePlaylist_;
}