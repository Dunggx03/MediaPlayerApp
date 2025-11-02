#pragma once
#include "utils/NcursesUI.h"
#include <string>
#include <vector>
#include <memory>

class SidebarView;
class BottomBarView;
class IMainAreaView;
class AppController;
class MediaFile; 
class PopupView; 
class TopBarView;

enum class AppMode {
    FILE_BROWSER,
    PLAYLISTS,
    USB_BROWSER
};

enum class FocusArea {
    SIDEBAR,
    MAIN_LIST,
    MAIN_DETAIL,
    BOTTOM_BAR
};

class UIManager {
public:
    explicit UIManager(NcursesUI* ui, AppController* controller);
    ~UIManager();
    bool init();
    void run();

private:
    void handleInput(InputEvent event);
    void switchMainView(AppMode newMode);
    WINDOW* getWindowAt(int globalY, int globalX, int& localY, int& localX);

    NcursesUI* ui;
    AppController* appController;
    bool isRunning;

    int screenH, screenW;
    int mainHeight, mainWidth;

    WINDOW *sidebarWin;
    WINDOW *mainWin;
    WINDOW *bottomWin; 
    WINDOW *topWin;
    
    int sidebarWidth;
    int bottomBarHeight;
    int topBarHeight;


    AppMode currentMode;
    FocusArea currentFocus;

    std::unique_ptr<SidebarView> sidebarView;
    std::unique_ptr<BottomBarView> bottomBarView;
    std::unique_ptr<IMainAreaView> mainAreaView;
    std::unique_ptr<TopBarView> topBarView;

    bool needsRedrawSidebar;
    bool needsRedrawMain;

    void showCreatePlaylistPopup();
    void showAddToPlaylistPopup(MediaFile* fileToAdd);
    
    std::unique_ptr<PopupView> popup; 

};