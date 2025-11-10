#include "controller/MediaController.h"
#include "model/MediaFile.h"
#include "model/Metadata.h"
#include "model/Playlist.h" 
#include <iostream> 

MediaController::MediaController(MediaManager* manager, MediaPlayer* player, 
                                 TagLibWrapper* tagUtil, DeviceConnector* device)
    : mediaManager(manager), 
      mediaPlayer(player), 
      tagUtil(tagUtil), 
      deviceConnector(device) 
{
    if (!manager || !player || !tagUtil || !device) {
        std::cerr << "CRITICAL: MediaController initialized with null pointers!" << std::endl;
    }
    std::cout << "MediaController: Initialized." << std::endl;
}


void MediaController::playTrack(MediaFile* file) {
    std::cout << "MediaController: playTrack called for " << (file ? file->getFileName() : "nullptr") << std::endl;
    if (mediaPlayer && file) {
        mediaPlayer->play(file, nullptr);
    } else {
        std::cerr << "MediaController: Cannot play track (null player or file)." << std::endl;
    }
}

void MediaController::pauseOrResume() {
    std::cout << "MediaController: pauseOrResume called." << std::endl;
    if (mediaPlayer) {
        mediaPlayer->pause();
    }
}

void MediaController::stop() {
    std::cout << "MediaController: stop called." << std::endl;
    if (mediaPlayer) {
        mediaPlayer->stop();
    }
}

void MediaController::setVolume(int volume) {
    std::cout << "MediaController: setVolume called with " << volume << std::endl;
     if (mediaPlayer) {
        mediaPlayer->setVolume(volume);
    }
}

void MediaController::loadMediaFromPath(const std::string& path) {
    std::cout << "MediaController: loadMediaFromPath called for " << path << std::endl;
     if (mediaManager) {
        mediaManager->loadFromDirectory(path);
    }
}

MediaFile* MediaController::findAdjacentTrack(int offset) {
    if (!mediaPlayer || !mediaManager) return nullptr;

    MediaFile* currentTrack = mediaPlayer->getCurrentTrack();
    if (!currentTrack) return nullptr;

    Playlist* activePlaylist = mediaPlayer->getActivePlaylist();
    if (activePlaylist) {
        std::cout << "[DEBUG] findAdjacentTrack: In Playlist context." << std::endl;
        const auto& tracks = activePlaylist->getTracks();
        if (tracks.empty()) return nullptr;

        auto it = std::find(tracks.begin(), tracks.end(), currentTrack);
        if (it == tracks.end()) {
             std::cerr << "ERROR: Current track not found in active playlist!" << std::endl;
             return nullptr;
        }
        
        int currentIndex = std::distance(tracks.begin(), it);
        int totalTracks = tracks.size();
        
        int nextIndex = (currentIndex + offset + totalTracks) % totalTracks;
        
        return tracks[nextIndex];
        
    } else {
        std::cout << "[DEBUG] findAdjacentTrack: In Library context (inefficient)." << std::endl;
        int totalFiles = mediaManager->getTotalFileCount();
        if (totalFiles <= 1) return nullptr;

        int currentIndex = -1;
        int pageSize = 25; 
        int totalPages = mediaManager->getTotalPages(pageSize);
        for (int p = 1; p <= totalPages; ++p) {
            std::vector<MediaFile*> page = mediaManager->getPage(p, pageSize);
            for(size_t i = 0; i < page.size(); ++i) {
                if (page[i] == currentTrack) {
                    currentIndex = (p - 1) * pageSize + i;
                    goto found_index_library;
                }
            }
        }

    found_index_library:
        if (currentIndex == -1) return nullptr;
        int nextIndex = (currentIndex + offset + totalFiles) % totalFiles;
        int targetPage = (nextIndex / pageSize) + 1;
        int indexOnPage = nextIndex % pageSize;
        std::vector<MediaFile*> targetPageData = mediaManager->getPage(targetPage, pageSize);
        if (indexOnPage >= 0 && (size_t)indexOnPage < targetPageData.size()) {
            return targetPageData[indexOnPage];
        }
    }
    
    return nullptr;
}


// --- Playback Control Implementations ---
void MediaController::nextTrack() {
    std::cout << "MediaController: nextTrack called." << std::endl;
    MediaFile* next = findAdjacentTrack(1); // Get the next track (+1 offset)
    if (next) {
        if (mediaPlayer->getActivePlaylist()) {
            mediaPlayer->play(next, mediaPlayer->getActivePlaylist());
        } else {
            mediaPlayer->play(next, nullptr);
        }
    } else {
        std::cout << "MediaController: Could not find next track." << std::endl;
    }
}

void MediaController::previousTrack() {
    std::cout << "MediaController: previousTrack called." << std::endl;
    MediaFile* prev = findAdjacentTrack(-1); // Get the previous track (-1 offset)
    if (prev) {
        if (mediaPlayer->getActivePlaylist()) {
            mediaPlayer->play(prev, mediaPlayer->getActivePlaylist());
        } else {
            mediaPlayer->play(prev, nullptr);
        }    
    } else {
        std::cout << "MediaController: Could not find previous track." << std::endl;
        // Optional: Stop playback or restart current? mediaPlayer->stop();
    }
}


void MediaController::increaseVolume(int amount) {
    std::cout << "MediaController: increaseVolume called." << std::endl;
    if (mediaPlayer) {
        int currentVol = mediaPlayer->getVolume();
        setVolume(std::min(100, currentVol + amount)); // Use setVolume, clamp at 100
    }
}

void MediaController::decreaseVolume(int amount) {
    std::cout << "MediaController: decreaseVolume called." << std::endl;
    if (mediaPlayer) {
        int currentVol = mediaPlayer->getVolume();
        setVolume(std::max(0, currentVol - amount)); // Use setVolume, clamp at 0
    }
}

void MediaController::playPlaylist(Playlist* playlist, int startIndex) {
    if (!mediaPlayer || !playlist) {
        std::cerr << "MediaController: Cannot play playlist (null player or playlist)." << std::endl;
        return;
    }
    
    const auto& tracks = playlist->getTracks();
    if (tracks.empty()) {
        std::cerr << "MediaController: Playlist is empty." << std::endl;
        return;
    }

    if (startIndex < 0 || (size_t)startIndex >= tracks.size()) {
        startIndex = 0;
    }

    MediaFile* fileToPlay = tracks[startIndex];
    
    std::cout << "MediaController: playPlaylist called for '" << playlist->getName() 
              << "', starting at track " << startIndex << std::endl;
              
    // call play, input playlist as context
    mediaPlayer->play(fileToPlay, playlist); 
}

bool MediaController::saveMetadataChanges(MediaFile* file) {
    if (!file || !file->getMetadata() || !tagUtil) {
        std::cerr << "MediaController: Cannot save metadata (null pointers)." << std::endl;
        return false;
    }
    
    Metadata* meta = file->getMetadata();
    
    bool success = tagUtil->writeTags(file->getFilePath(), meta);
    
    if (!success) {
       std::cerr << "MediaController: Failed to write tags to file: " << file->getFilePath() << std::endl;
    }
    return success;
}

void MediaController::onDevicePlayPause() {
    std::cout << "MediaController: onDevicePlayPause received." << std::endl;
    pauseOrResume();
}
void MediaController::onDeviceNext() { std::cout << "MediaController: (Stub) onDeviceNext.\n"; }
void MediaController::onDevicePrevious() { std::cout << "MediaController: (Stub) onDevicePrevious.\n"; }
void MediaController::onDeviceVolumeChange(int adcValue) { std::cout << "MediaController: (Stub) onDeviceVolumeChange: " << adcValue << "\n"; }

void MediaController::sendSongInfoToDevice(MediaFile* file) {}
int MediaController::convertAdcToVolume(int adcValue) { return 50; }

void MediaController::setUSBMediaManager(MediaManager* usbMgr) { usbMediaManager = usbMgr; }
