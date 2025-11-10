#include "SDLWrapper.h"
#include <iostream>

// Initialize the static callback
std::function<void()> SDLWrapper::s_onTrackFinishedCallback = nullptr;

SDLWrapper::SDLWrapper() : isInitialized(false), currentMusic(nullptr) {}

SDLWrapper::~SDLWrapper() {
    close();
}

// Static function for SDL_mixer to call
void SDLWrapper::musicFinishedCallback() {
    std::cout << "DEBUG SDLWrapper: musicFinishedCallback() triggered." << std::endl;
    if (s_onTrackFinishedCallback) {
        s_onTrackFinishedCallback(); // Call the C++ function (MediaPlayer::onTrackFinished)
    }
}

bool SDLWrapper::init() {
    if (isInitialized) return true;
    std::cout << "DEBUG SDLWrapper: Initializing SDL Audio..." << std::endl;
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "ERROR SDLWrapper: Could not initialize SDL. " << SDL_GetError() << std::endl;
        return false;
    }
    std::cout << "DEBUG SDLWrapper: Initializing SDL_mixer..." << std::endl;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "ERROR SDLWrapper: Could not initialize SDL_mixer. " << Mix_GetError() << std::endl;
        SDL_Quit();
        return false;
    }
    Mix_AllocateChannels(16);
    Mix_HookMusicFinished(SDLWrapper::musicFinishedCallback); // Register callback
    isInitialized = true;
    std::cout << "DEBUG SDLWrapper: Initialization successful." << std::endl;
    return true;
}

void SDLWrapper::close() {
    if (!isInitialized) return;
    std::cout << "DEBUG SDLWrapper: Closing..." << std::endl;
    stopAudio(); // Halt and free music first
    Mix_HookMusicFinished(nullptr); // Unregister callback
    Mix_CloseAudio();
    Mix_Quit();
    SDL_Quit();
    isInitialized = false;
    std::cout << "DEBUG SDLWrapper: Closed successfully." << std::endl;
}

bool SDLWrapper::playAudio(const std::string& filePath) {
    if (!isInitialized) {
         std::cerr << "ERROR SDLWrapper: playAudio called but not initialized!" << std::endl;
        return false;
    }
    std::cout << "DEBUG SDLWrapper: playAudio requested for: " << filePath << std::endl;

    stopAudio(); // Stop and free previous music

    std::cout << "DEBUG SDLWrapper: Loading music with Mix_LoadMUS..." << std::endl;
    currentMusic = Mix_LoadMUS(filePath.c_str());
    if (currentMusic == nullptr) {
        std::cerr << "ERROR SDLWrapper: Failed to load music: '" << filePath << "' - " << Mix_GetError() << std::endl;
        return false;
    }
    std::cout << "DEBUG SDLWrapper: Music loaded successfully." << std::endl;


    std::cout << "DEBUG SDLWrapper: Calling Mix_PlayMusic..." << std::endl;
    if (Mix_PlayMusic(currentMusic, 1) == -1) { // Play 1 time
        std::cerr << "ERROR SDLWrapper: Failed to play music: " << Mix_GetError() << std::endl;
        Mix_FreeMusic(currentMusic);
        currentMusic = nullptr;
        return false;
    }

    std::cout << "DEBUG SDLWrapper: Mix_PlayMusic call succeeded. Playback started." << std::endl;
    return true;
}

void SDLWrapper::pauseAudio() {
    if (!isInitialized) return;

    // Check if music is actually loaded and playing/paused
    if (currentMusic && (Mix_PlayingMusic() || Mix_PausedMusic()) ) {
        if (Mix_PausedMusic()) {
            std::cout << "DEBUG SDLWrapper: Resuming music." << std::endl;
            Mix_ResumeMusic();
        } else {
            std::cout << "DEBUG SDLWrapper: Pausing music." << std::endl;
            Mix_PauseMusic();
        }
    } else {
         std::cout << "DEBUG SDLWrapper: pauseAudio called but no music playing/paused." << std::endl;
    }
}

void SDLWrapper::stopAudio() {
    if (!isInitialized) return;
    // Check if music is playing before halting (optional, Mix_HaltMusic is safe)
    if(Mix_PlayingMusic() || Mix_PausedMusic()){
        std::cout << "DEBUG SDLWrapper: Halting music." << std::endl;
        Mix_HaltMusic();
    }

    if (currentMusic != nullptr) {
        std::cout << "DEBUG SDLWrapper: Freeing current music resource." << std::endl;
        Mix_FreeMusic(currentMusic);
        currentMusic = nullptr;
    }
}

int SDLWrapper::getCurrentTime() const {
    // Add null check for robustness, although Mix_PlayingMusic should suffice
    if (currentMusic && Mix_PlayingMusic()) {
        return static_cast<int>(Mix_GetMusicPosition(currentMusic));
    }
    return 0;
}

void SDLWrapper::setTrackFinishedCallback(std::function<void()> callback) {
    std::cout << "DEBUG SDLWrapper: Setting track finished callback." << std::endl;
    s_onTrackFinishedCallback = callback;
}