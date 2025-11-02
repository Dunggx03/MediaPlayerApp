#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include "MediaFile.h"

class MediaPlayer {
public:
    using Callback = std::function<void(const std::string& event, const MediaFile* file)>;

    MediaPlayer();
    ~MediaPlayer();

    void setPlaylistAndPlay(const std::vector<MediaFile*>& newList, size_t index);
    void play(size_t index);
    void pause();
    void resume();
    void stop();
    void next();
    void previous();

    void setVolume(float value);
    float getVolume() const;

    void setCallback(Callback cb);

    float getCurrentTime() const;
    const MediaFile* getCurrentTrack() const;

    const std::vector<MediaFile*>& getCurrentPlaylist() const { return playlist; }
    size_t getCurrentIndex() const { return currentIndex; }

private:
    void playbackThread(MediaFile* file);
    void autoNextWhenFinish();
    void cleanup();

    std::vector<MediaFile*> playlist;
    size_t currentIndex = 0;

    std::thread playThread;
    std::mutex mtx;
    std::atomic<bool> isPlaying{false};
    std::atomic<bool> stopRequested{false};
    std::atomic<float> volume{1.0f};

    Callback callback;

    Mix_Music* currentMusic = nullptr;
    std::atomic<float> currentTime{0.0f};
};

#endif // MEDIA_PLAYER_H
