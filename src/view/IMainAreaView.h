#pragma once
#include "utils/NcursesUI.h"
#include "view/UIManager.h" 

enum class MainAreaAction {
    NONE, 
    
    // Playlist Actions
    CREATE_PLAYLIST,
    DELETE_PLAYLIST,
    PLAY_PLAYLIST,
    REMOVE_TRACK_FROM_PLAYLIST,
    
    // File Actions
    ADD_TRACK_TO_PLAYLIST,
    EDIT_METADATA 
};

class IMainAreaView {
public:
    virtual ~IMainAreaView() = default;
    virtual void draw(FocusArea focus) = 0;

    virtual MainAreaAction handleInput(InputEvent event, FocusArea focus) = 0;
    virtual MainAreaAction handleMouse(int localY, int localX) = 0;
};