#pragma once
#include <memory>
#include <functional>
#include "MediaFile.h"
#include "utils/SDLWrapper.h"

class Playlist;

enum class PlayerState { STOPPED, PLAYING, PAUSED };

class MediaPlayer {
public:
    MediaPlayer(SDLWrapper* sdlWrapper);

    void play(MediaFile* file, Playlist* context = nullptr);
    void pause(); // Toggles pause/resume
    void stop();

    void setVolume(int volume); // 0-100
    int getVolume() const;

    PlayerState getState() const;
    int getCurrentTime() const; 
    int getTotalTime() const;   
    MediaFile* getCurrentTrack() const;
    
    void onTrackFinished(); 
    void setOnTrackFinishedCallback(std::function<void()> callback);

    Playlist* getActivePlaylist() const;
private:
    SDLWrapper* sdlWrapper;     
    MediaFile* currentTrack;  
    int currentVolume;

    PlayerState currentState;
    
    std::function<void()> onTrackFinishedCallback_;
    bool isStoppingManually_;

    Playlist* activePlaylist_;
};