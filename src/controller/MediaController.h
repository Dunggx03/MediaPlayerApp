#pragma once
#include "model/MediaManager.h"
#include "model/MediaPlayer.h"
#include "model/MediaFile.h"

#include "utils/DeviceConnector.h"
#include "utils/TagLibWrapper.h"

class Playlist;
class Metadata;

class MediaController {
public:
    MediaController(MediaManager* manager, MediaPlayer* player, 
                    TagLibWrapper* tagUtil, DeviceConnector* device);

    void playTrack(MediaFile* file);
    void pauseOrResume();
    void stop();
    void setVolume(int volume);
    void loadMediaFromPath(const std::string& path);
    bool saveMetadataChanges(MediaFile* file);

    void nextTrack();
    void previousTrack();
    void increaseVolume(int amount = 5);
    void decreaseVolume(int amount = 5);
    void setUSBMediaManager(MediaManager* usbMgr);
    void onDevicePlayPause();
    void onDeviceNext();
    void onDevicePrevious();
    void onDeviceVolumeChange(int adcValue); // Raw value from ADC
    void playPlaylist(Playlist* playlist, int startIndex = 0);
private:
    void sendSongInfoToDevice(MediaFile* file);
    
    int convertAdcToVolume(int adcValue);

    MediaManager* mediaManager;
    MediaManager* usbMediaManager = nullptr; 
    MediaPlayer* mediaPlayer;
    MediaFile* findAdjacentTrack(int offset); 
    TagLibWrapper* tagUtil;
    DeviceConnector* deviceConnector;
};