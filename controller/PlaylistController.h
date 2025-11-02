#ifndef PLAYLIST_CONTROLLER_H
#define PLAYLIST_CONTROLLER_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "../model/PlaylistManager.h"
#include "../model/MediaManager.h"
#include "../model/MediaPlayer.h"

class PlaylistController {
public:
    using ViewCallback = std::function<void(const std::string& event, const std::string& playlistName)>;

    PlaylistController(MediaManager* mediaMgr, MediaPlayer* player);
    ~PlaylistController() = default;

    // CRUD playlist
    bool createPlaylist(const std::string& name);
    bool deletePlaylist(const std::string& name);

    // Quản lý track
    bool addToPlaylist(const std::string& playlistName, const std::string& mediaPath);
    bool removeFromPlaylist(const std::string& playlistName, const std::string& trackName);

    // Xem và phát playlist
    std::vector<std::string> viewPlaylist(const std::string& name);
    void playPlaylist(const std::string& name, size_t startIndex = 0);

    // Quản lý callback cho View
    void setViewCallback(ViewCallback cb);

    // Truy xuất danh sách playlist hiện có
    std::vector<std::string> getAllPlaylists() const;

private:
    PlaylistManager playlistManager;
    MediaManager* mediaManager;   // dùng để kiểm tra file tồn tại
    MediaPlayer* mediaPlayer;     // để phát nhạc từ playlist

    ViewCallback viewCallback;

    void notifyView(const std::string& event, const std::string& name);
};

#endif
