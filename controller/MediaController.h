#ifndef MEDIA_CONTROLLER_H
#define MEDIA_CONTROLLER_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "../model/MediaManager.h"
#include "../model/MediaPlayer.h"
#include "../model/PlaylistManager.h"

class MediaController {
public:
    using ViewCallback = std::function<void(const std::string& event, const MediaFile* file)>;

    MediaController();
    ~MediaController();

    // 🔹 Nạp nguồn dữ liệu (từ local hoặc USB)
    bool handleLoadSource(const std::string& sourceType);

    // 🔹 Playback control
    void playFile(const std::string& path);
    void playFromIndex(size_t index);
    void pause();
    void resume();
    void stop();
    void next();
    void previous();
    void setVolume(float value);

    // 🔹 Lấy danh sách file hiện tại
    std::vector<MediaFile*> getCurrentPage(int pageNum);
    int getTotalPages() const;

    // 🔹 Liên kết callback để View cập nhật UI
    void setViewCallback(ViewCallback cb);

    // 🔹 Kiểm tra trạng thái hiện tại
    const MediaFile* getCurrentTrack() const;
    float getVolume() const;

private:
    MediaManager mediaManager;
    MediaPlayer mediaPlayer;
    PlaylistManager playlistManager;

    std::string localPath;
    std::string usbPath;

    ViewCallback viewCallback;

    void onPlayerEvent(const std::string& event, const MediaFile* file);
    bool copyFromUSB(const std::string& mountPoint, const std::string& destPath);
};

#endif
