#pragma once
#include <string>
#include <functional>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

class SDLWrapper {
public:
    SDLWrapper();
    ~SDLWrapper();

    bool init();
    void close();

    bool playAudio(const std::string& filePath);
    void pauseAudio(); // This will toggle pause/resume
    void stopAudio();

    int getCurrentTime() const;

    void setTrackFinishedCallback(std::function<void()> callback);

private:
    static void musicFinishedCallback();

    bool isInitialized;
    Mix_Music* currentMusic;
    
    // Static callback pointer to our C++ function
    static std::function<void()> s_onTrackFinishedCallback;
};