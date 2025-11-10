#pragma once
#include "utils/NcursesUI.h"
#include "model/MediaPlayer.h"

enum class BottomBarAction {
    NONE,
    TOGGLE_PLAY_PAUSE,
    NEXT_TRACK,
    PREV_TRACK,
    VOLUME_UP,
    VOLUME_DOWN
};

class BottomBarView {
public:
    BottomBarView(NcursesUI* ui, MediaPlayer* player, WINDOW* win);
    void draw(bool hasFocus);
    BottomBarAction handleMouse(int localY, int localX);
    BottomBarAction handleKeyboard(int key);

private:
    NcursesUI* ui;
    MediaPlayer* player;
    WINDOW* win;

    // Store calculated button positions for click detection
    int prevX_start, prevX_end;
    int playPauseX_start, playPauseX_end;
    int nextX_start, nextX_end;
    int volDownX_start, volDownX_end;
    int volUpX_start, volUpX_end;
};