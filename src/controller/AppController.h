#pragma once
#include <memory> 
#include <string>
class MediaManager;
class PlaylistManager;
class MediaPlayer;
class MediaController;
class PlaylistController;

class TagLibWrapper;
class SDLWrapper;
class DeviceConnector;
class USBUtils;

class AppController {
public:
    AppController();
    ~AppController(); 

    bool init();

    MediaManager* getMediaManager() const;
    PlaylistManager* getPlaylistManager() const;
    MediaPlayer* getMediaPlayer() const;
    MediaController* getMediaController() const;
    PlaylistController* getPlaylistController() const;
    MediaManager* getUSBMediaManager() const;
    MediaController* getusbmediaController() const;
    bool loadUSBLibrary();
    bool ejectUSB();
    public:


private:
    std::string currentUSBPath;
    // --- Ownership Utils ---
    std::unique_ptr<TagLibWrapper> tagLibWrapper;
    std::unique_ptr<SDLWrapper> sdlWrapper;
    std::unique_ptr<DeviceConnector> deviceConnector;
    std::unique_ptr<USBUtils> usbUtils;

    // --- Ownership Model ---
    std::unique_ptr<MediaManager> mediaManager;
    std::unique_ptr<PlaylistManager> playlistManager;
    std::unique_ptr<MediaPlayer> mediaPlayer;
    std::unique_ptr<MediaManager> usbMediaManager;

    // --- Ownership Controller ---
    std::unique_ptr<MediaController> mediaController;
    std::unique_ptr<MediaController> usbmediaController;
    std::unique_ptr<PlaylistController> playlistController;
};
