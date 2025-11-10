#include "view/UIManager.h"
#include <iostream>
#include <cmath>
#include <tuple>
#include <vector> 

#include "controller/AppController.h"
#include "controller/MediaController.h"
#include "controller/PlaylistController.h"

#include "view/SidebarView.h"
#include "view/BottomBarView.h"
#include "view/MainFileView.h"
#include "view/MainPlaylistView.h"
#include "view/PopupView.h"
#include "view/TopBarView.h"
#include "model/MediaFile.h"
#include "model/Playlist.h"
#include "model/PlaylistManager.h"
#include "view/PopupView.h"
#include "view/MainUSBView.h"

UIManager::UIManager(NcursesUI* ui, AppController* controller)
    : ui(ui), appController(controller), isRunning(false),
      screenH(0), screenW(0), mainHeight(0), mainWidth(0),
      sidebarWin(nullptr), mainWin(nullptr), bottomWin(nullptr), topWin(nullptr),
      sidebarWidth(20), bottomBarHeight(5), topBarHeight(3),
      currentMode(AppMode::FILE_BROWSER),
      currentFocus(FocusArea::SIDEBAR),
      // Initialize redraw flags
      needsRedrawSidebar(true),
      needsRedrawMain(true),
      popup(nullptr) // Initialize popup pointer
{
    if (appController == nullptr) {
        std::cerr << "CRITICAL: UIManager started with null AppController!" << std::endl;
    }
}

UIManager::~UIManager() {}

bool UIManager::init() {
    if (ui == nullptr) return false;
    ui->getScreenDimensions(screenH, screenW);

    mainHeight = screenH - bottomBarHeight - topBarHeight;
    mainWidth = screenW - sidebarWidth;

    if (mainHeight < 10 || mainWidth < 20) return false;

    topWin = newwin(topBarHeight, screenW, 0, 0);
    sidebarWin = newwin(mainHeight, sidebarWidth, topBarHeight, 0);
    mainWin = newwin(mainHeight, mainWidth, topBarHeight, sidebarWidth);
    bottomWin = newwin(bottomBarHeight, screenW, topBarHeight + mainHeight, 0);

    if (!topWin || !sidebarWin || !mainWin || !bottomWin) return false;


    topBarView = std::make_unique<TopBarView>(ui, topWin, "🎵      OUR MEDIA PLAYER     🎵");
    sidebarView = std::make_unique<SidebarView>(ui, sidebarWin);
    bottomBarView = std::make_unique<BottomBarView>(ui, appController->getMediaPlayer(), bottomWin);
    switchMainView(AppMode::FILE_BROWSER);

    popup = std::make_unique<PopupView>(ui, screenH, screenW);
    return true;
}


void UIManager::run() {
    isRunning = true;
    while (isRunning) {
        InputEvent event = ui->getInput();

        if (event.type != InputEvent::UNKNOWN)
            handleInput(event);

        if (topBarView) topBarView->draw();
        if (needsRedrawSidebar && sidebarView) {
            sidebarView->draw(currentFocus == FocusArea::SIDEBAR);
            needsRedrawSidebar = false;
        }
        if (needsRedrawMain && mainAreaView) {
            mainAreaView->draw(currentFocus);
            needsRedrawMain = false;
        }
        if (bottomBarView)
            bottomBarView->draw(currentFocus == FocusArea::BOTTOM_BAR);

        doupdate();
    }
}


bool is_mouse_in_window(WINDOW* win, int globalY, int globalX) {
    if (!win) return false; int begy=getbegy(win), begx=getbegx(win), maxy=begy+getmaxy(win), maxx=begx+getmaxx(win);
    return (globalY >= begy && globalY < maxy && globalX >= begx && globalX < maxx);
}
WINDOW* UIManager::getWindowAt(int gY, int gX, int& lY, int& lX) {
    if (is_mouse_in_window(sidebarWin, gY, gX)) { lY=gY-getbegy(sidebarWin); lX=gX-getbegx(sidebarWin); return sidebarWin; }
    if (is_mouse_in_window(mainWin, gY, gX)) { lY=gY-getbegy(mainWin); lX=gX-getbegx(mainWin); return mainWin; }
    if (is_mouse_in_window(bottomWin, gY, gX)) { lY=gY-getbegy(bottomWin); lX=gX-getbegx(bottomWin); return bottomWin; }
    return nullptr;
}


void UIManager::handleInput(InputEvent event) {
     if (event.type == InputEvent::UNKNOWN) return;

     FocusArea oldFocus = currentFocus;
     MainAreaAction mainAction = MainAreaAction::NONE;
     BottomBarAction bottomAction = BottomBarAction::NONE;

     // --- MOUSE HANDLING ---
     if (event.type == InputEvent::MOUSE) {
        int localY, localX;
        WINDOW* clickedWin = getWindowAt(event.mouseY, event.mouseX, localY, localX);

        if (clickedWin == sidebarWin && sidebarView) {
            currentFocus = FocusArea::SIDEBAR;
            AppMode newMode = sidebarView->handleMouse(localY, localX);
            if (newMode != currentMode) switchMainView(newMode);
            if (sidebarView->shouldExit()) isRunning = false;
            needsRedrawSidebar = true;
        }
        else if (clickedWin == mainWin && mainAreaView) {
            if (localX < mainWidth / 2) currentFocus = FocusArea::MAIN_LIST;
            else currentFocus = FocusArea::MAIN_DETAIL;
            mainAction = mainAreaView->handleMouse(localY, localX); // Get action
            needsRedrawMain = true;
        }
        else if (clickedWin == bottomWin && bottomBarView) {
            currentFocus = FocusArea::BOTTOM_BAR;
            // Get specific action from BottomBarView
            bottomAction = bottomBarView->handleMouse(localY, localX);
            needsRedrawSidebar = true; // Redraw other panels for focus
            needsRedrawMain = true;
        }
     }

     // --- KEYBOARD HANDLING ---
     else if (event.type == InputEvent::KEYBOARD) {
        if (event.key == 9) { // Tab key
            if (currentFocus == FocusArea::SIDEBAR) 
                currentFocus = FocusArea::MAIN_LIST; 
            else if (currentFocus == FocusArea::MAIN_LIST) 
                currentFocus = FocusArea::MAIN_DETAIL; 
            else if (currentFocus == FocusArea::MAIN_DETAIL) 
                currentFocus = FocusArea::BOTTOM_BAR; 
            else if (currentFocus == FocusArea::BOTTOM_BAR) 
                currentFocus = FocusArea::SIDEBAR;
            
                needsRedrawSidebar = true; needsRedrawMain = true;
            return;
        }

        switch (currentFocus) {
            case FocusArea::SIDEBAR:
                if (sidebarView) 
                { 
                    /* ... Sidebar key handling ... */ 
                    AppMode oldM=currentMode; 
                    AppMode newM=sidebarView->handleInput(event); 
                    if(newM != oldM) switchMainView(newM); 
                    if(sidebarView->shouldExit()) isRunning=false; 
                    needsRedrawSidebar=true; 
                }
                break;

            case FocusArea::MAIN_LIST:
                 if (mainAreaView) {
                     mainAction = mainAreaView->handleInput(event, currentFocus);
                     needsRedrawMain = true;
                 }
                 // Special handling for Enter key to play
                 if (event.key == 10) { /* ... Play track logic ... */ 
                    MediaFile* sf=nullptr; 
                    if(currentMode==AppMode::FILE_BROWSER){
                        MainFileView* fv=dynamic_cast<MainFileView*>(mainAreaView.get());
                        if(fv) {
                            sf=fv->getSelectedFile();}
                    }
                    else if(currentMode==AppMode::USB_BROWSER){
                        MainUSBView* uv=dynamic_cast<MainUSBView*>(mainAreaView.get());
                        if(uv) {
                            sf=uv->getSelectedFile();}
                    }
                     else if(currentMode==AppMode::PLAYLISTS){
                        MainPlaylistView* pv=dynamic_cast<MainPlaylistView*>(mainAreaView.get()); 
                        if(pv) sf=pv->getSelectedTrack();
                    } 
                    if(sf&&appController&&appController->getMediaController()){
                        appController->getMediaController()->playTrack(sf);
                    } 
                 else{
                    if(ui)flash();
                }
                }
                 break;

            case FocusArea::MAIN_DETAIL:
                 if (mainAreaView) {
                     mainAction = mainAreaView->handleInput(event, currentFocus);
                     needsRedrawMain = true;
                 }
                break;

            case FocusArea::BOTTOM_BAR:
                if (bottomBarView) {
                    // Get action from BottomBarView's keyboard handler
                    bottomAction = bottomBarView->handleKeyboard(event.key);
                }
                needsRedrawSidebar = true; // Redraw focus highlights
                needsRedrawMain = true;
                break;
        }
     }


    // Handle Main Area Actions first
    switch (mainAction) 
    {
        case MainAreaAction::CREATE_PLAYLIST: showCreatePlaylistPopup(); break;
        case MainAreaAction::DELETE_PLAYLIST: 
        { 
            /* ... Delete logic ... */ 
            Playlist* sp=nullptr; 
            if(currentMode==AppMode::PLAYLISTS)
            {
                MainPlaylistView* pv=dynamic_cast<MainPlaylistView*>(mainAreaView.get()); 
                if(pv) sp=pv->getSelectedPlaylist();
            } 
            if(sp&&appController&&appController->getPlaylistController())
            {
                bool d=appController->getPlaylistController()->deletePlaylist(sp->getName()); 
                if(d)needsRedrawMain=true; else flash();
            } 
            else 
            flash();
        } 
        break;
        case MainAreaAction::REMOVE_TRACK_FROM_PLAYLIST: 
        { 
            /* ... Remove logic ... */ 
            MediaFile* st=nullptr; 
            Playlist* cp=nullptr; 
            if(currentMode==AppMode::PLAYLISTS)
            {
                MainPlaylistView* pv=dynamic_cast<MainPlaylistView*>(mainAreaView.get()); 
                if(pv){st=pv->getSelectedTrack(); 
                    cp=pv->getSelectedPlaylist();
                }
            } 
            if(st&&cp&&appController&&appController->getPlaylistController())
            {
                bool r=appController->getPlaylistController()->removeTrackFromPlaylist(st,cp); 
                if(r)needsRedrawMain=true; 
                else flash();
            } 
            else  flash();
        } break;
        case MainAreaAction::ADD_TRACK_TO_PLAYLIST: 
        {
            MediaFile* sf = nullptr; 

            if (currentMode == AppMode::FILE_BROWSER) {
                if (auto* fv = dynamic_cast<MainFileView*>(mainAreaView.get()))
                    sf = fv->getSelectedFile();
            } 
            else if (currentMode == AppMode::USB_BROWSER) {
                if (auto* uv = dynamic_cast<MainUSBView*>(mainAreaView.get()))
                    sf = uv->getSelectedFile();
            }

            if (sf)
                showAddToPlaylistPopup(sf);
            else
                flash();
        }
        break;
        case MainAreaAction::EDIT_METADATA: 
        {
            if (currentMode != AppMode::FILE_BROWSER && currentMode != AppMode::USB_BROWSER)
                break;

            MediaFile* fileToEdit = nullptr;

            if (currentMode == AppMode::FILE_BROWSER) {
                if (auto* fv = dynamic_cast<MainFileView*>(mainAreaView.get()))
                    fileToEdit = fv->getSelectedFile();
            }
            else if (currentMode == AppMode::USB_BROWSER) {
                if (auto* uv = dynamic_cast<MainUSBView*>(mainAreaView.get()))
                    fileToEdit = uv->getSelectedFile();
            }

            if (!fileToEdit || !popup) {
                flash();
                break;
            }

            bool saved = popup->showMetadataEditor(fileToEdit->getMetadata());

            needsRedrawSidebar = true;
            needsRedrawMain = true;

            if (saved && appController && appController->getMediaController()) {
                appController->getMediaController()->saveMetadataChanges(fileToEdit);
            }

            break;
        }

        case MainAreaAction::PLAY_PLAYLIST:
            {
                if (currentMode == AppMode::PLAYLISTS) {
                    auto plView = dynamic_cast<MainPlaylistView*>(mainAreaView.get());
                    if (plView && appController && appController->getMediaController()) {
                        Playlist* selectedPlaylist = plView->getSelectedPlaylist();
                        int trackIndex = plView->getSelectedTrackIndex();
                        
                        if (selectedPlaylist) {
                            appController->getMediaController()->playPlaylist(selectedPlaylist, trackIndex);
                        } else {
                            flash();
                        }
                    }
                }
                break;
            }
         case MainAreaAction::NONE: default: break;
    }

    if(currentMode == AppMode::FILE_BROWSER)
    {
        if (bottomAction != BottomBarAction::NONE && appController && appController->getMediaController()) {
            MediaController* mc = appController->getMediaController();
            switch (bottomAction) {
                case BottomBarAction::TOGGLE_PLAY_PAUSE: mc->pauseOrResume(); break;
                case BottomBarAction::NEXT_TRACK: mc->nextTrack(); break;
                case BottomBarAction::PREV_TRACK: mc->previousTrack(); break;
                case BottomBarAction::VOLUME_UP: mc->increaseVolume(); break;
                case BottomBarAction::VOLUME_DOWN: mc->decreaseVolume(); break;
                case BottomBarAction::NONE: default: break; 
            }
        }
    }
    else if (currentMode == AppMode::USB_BROWSER)
    {
    if (bottomAction != BottomBarAction::NONE && appController && appController->getusbmediaController()) {
            MediaController* mc = appController->getusbmediaController();
            switch (bottomAction) {
                case BottomBarAction::TOGGLE_PLAY_PAUSE: mc->pauseOrResume(); break;
                case BottomBarAction::NEXT_TRACK: mc->nextTrack(); break;
                case BottomBarAction::PREV_TRACK: mc->previousTrack(); break;
                case BottomBarAction::VOLUME_UP: mc->increaseVolume(); break;
                case BottomBarAction::VOLUME_DOWN: mc->decreaseVolume(); break;
                case BottomBarAction::NONE: default: break; // Should not happen here
            }
        }
    }
    else if (currentMode == AppMode::PLAYLISTS)
    {
    if (bottomAction != BottomBarAction::NONE && appController && appController->getusbmediaController()) {
            MediaController* mc = appController->getusbmediaController();
            switch (bottomAction) {
                case BottomBarAction::TOGGLE_PLAY_PAUSE: mc->pauseOrResume(); break;
                case BottomBarAction::NEXT_TRACK: mc->nextTrack(); break;
                case BottomBarAction::PREV_TRACK: mc->previousTrack(); break;
                case BottomBarAction::VOLUME_UP: mc->increaseVolume(); break;
                case BottomBarAction::VOLUME_DOWN: mc->decreaseVolume(); break;
                case BottomBarAction::NONE: default: break; // Should not happen here
            }
        }
    }



     if(oldFocus != currentFocus) {
         needsRedrawSidebar = true;
         needsRedrawMain = true;
     }
}


void UIManager::showCreatePlaylistPopup() {
    if (!popup) return; 
    std::optional<std::string> name = popup->showTextInput("Enter New Playlist Name:"); 
    needsRedrawSidebar=true; 
    needsRedrawMain=true; 
    if(name.has_value()&&!name.value().empty()&&appController&&appController->getPlaylistController())
    {
        bool s=appController->getPlaylistController()->createPlaylist(name.value()); 
        if(!s)flash(); 
        needsRedrawMain=true;
    }
}

void UIManager::showAddToPlaylistPopup(MediaFile* fileToAdd) {
    if(!fileToAdd||!appController||!appController->getPlaylistManager()) return; 
    std::vector<Playlist*> pls=appController->getPlaylistManager()->getAllPlaylists(); 
    std::vector<std::string> opts; 
    for(const auto* p:pls) opts.push_back(p->getName()); 
    if(opts.empty())
    {
        flash();
        return;
    } 
    if(!popup)return; 
    std::optional<int> sel=popup->showListSelection("Add to Playlist:", opts); 
    needsRedrawSidebar=true; 
    needsRedrawMain=true; 
    if(sel.has_value())
    {
        int idx=sel.value(); 
        if(idx>=0&&static_cast<size_t>(idx)<pls.size())
        {
            Playlist* sp=pls[idx]; 
            if(sp&&appController->getPlaylistController())
            {
                bool a=appController->getPlaylistController()->addTrackToPlaylist(fileToAdd,sp); 
                if(!a) flash();
            }
        }
    }
}

void UIManager::switchMainView(AppMode newMode) {
    if (newMode == currentMode && mainAreaView != nullptr) return;
    currentMode = newMode;

    if (newMode == AppMode::FILE_BROWSER) {
        mainAreaView = std::make_unique<MainFileView>(ui, mainWin, appController->getMediaManager());
    }
    else if (newMode == AppMode::USB_BROWSER) {
    mainAreaView = std::make_unique<MainUSBView>(ui, mainWin, appController);
}

    else if (newMode == AppMode::PLAYLISTS) {
        mainAreaView = std::make_unique<MainPlaylistView>(ui, mainWin, appController->getPlaylistManager());
    }
    else {
        mainAreaView = nullptr;
    }
    needsRedrawMain = true;
}
