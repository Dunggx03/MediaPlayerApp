#ifndef CONTROLLER_MANAGER_H
#define CONTROLLER_MANAGER_H

#include <string>
#include <memory>
#include <functional>
#include "../controller/MediaController.h"
#include "../controller/PlaylistController.h"
#include "../model/MediaManager.h"
#include "../model/MediaPlayer.h"

enum class ActiveMenu {
    MEDIA_MENU,
    PLAYLIST_MENU
};

class ControllerManager {
public:
    using ViewCallback = std::function<void(const std::string& event, const std::string& info)>;

    ControllerManager();
    ~ControllerManager();

    // 🔹 Bắt đầu chương trình
    void start();

    // 🔹 Chuyển đổi menu
    void switchToMediaMenu();
    void switchToPlaylistMenu();

    // 🔹 Gán callback để View nhận thông báo
    void setViewCallback(ViewCallback cb);

    // 🔹 Truy xuất Controller con
    MediaController& getMediaController();
    PlaylistController& getPlaylistController();

    // 🔹 Gửi event từ View đến hệ thống
    void handleUserAction(const std::string& action, const std::string& argument = "");

private:
    std::unique_ptr<MediaManager> mediaManager;
    std::unique_ptr<MediaPlayer> mediaPlayer;

    std::unique_ptr<MediaController> mediaController;
    std::unique_ptr<PlaylistController> playlistController;

    ActiveMenu currentMenu;
    ViewCallback viewCallback;

    void relayEventToView(const std::string& event, const std::string& info);
};

#endif
