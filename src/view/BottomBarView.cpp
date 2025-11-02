#include "view/BottomBarView.h"
#include "model/MediaFile.h"
#include <string>
#include <vector>
#include <algorithm> 
#include <cstring>   

BottomBarView::BottomBarView(NcursesUI* ui, MediaPlayer* player, WINDOW* win)
    : ui(ui), player(player), win(win),
      prevX_start(0), prevX_end(0),
      playPauseX_start(0), playPauseX_end(0),
      nextX_start(0), nextX_end(0),
      volDownX_start(0), volDownX_end(0),
      volUpX_start(0), volUpX_end(0)
{}

void BottomBarView::draw(bool hasFocus) {
    werase(win);
    box(win, 0, 0);

    std::string title = "Stopped";
    int currentTime = 0;
    int totalTime = 0;
    PlayerState state = PlayerState::STOPPED;

    // --- GET REAL DATA ---
    if (player != nullptr) {
        state = player->getState();
        if (state != PlayerState::STOPPED && player->getCurrentTrack()) {
            title = player->getCurrentTrack()->getFileName();
            currentTime = player->getCurrentTime();
            totalTime = player->getTotalTime();
        }
    }
    if (totalTime <= 0) totalTime = 1; // Avoid division by zero

    // --- FORMAT TIME ---
    char timeStr[30];
    sprintf(timeStr, "%02d:%02d / %02d:%02d",
            currentTime / 60, currentTime % 60,
            totalTime / 60, totalTime % 60);

    // --- CALCULATE PROGRESS BAR ---
    int width = getmaxx(win);
    int barWidth = width - 4; if(barWidth < 1) barWidth = 1;
    int progressChars = 0;
    if (totalTime > 0 && currentTime >= 0) {
        progressChars = static_cast<int>( (static_cast<double>(currentTime) / totalTime) * barWidth );
        progressChars = std::max(0, std::min(progressChars, barWidth -1));
    }
    std::string progressBar(barWidth, '-');
    if (barWidth > 0 && progressChars < barWidth) progressBar[progressChars] = 'o';

    // --- DRAW COMPONENTS ---
    // Line 1: Title and Time
    mvwprintw(win, 1, 2, "Now Playing: %.*s", width - 25, title.c_str());
    mvwprintw(win, 1, width - strlen(timeStr) - 2, "%s", timeStr);

    // Line 2: Progress Bar
    mvwprintw(win, 2, 2, "%s", progressBar.c_str());

    // --- Line 3: Controls (Calculate and Store Positions) ---
    const char* prevLabel = "<<";
    const char* playPauseIcon = (state == PlayerState::PLAYING) ? "II" : "> ";
    const char* nextLabel = ">>";
    const char* volDownLabel = "Vol-";
    const char* volUpLabel = "Vol+";
    int spacing = 3;

    int pbWidth = strlen(prevLabel) + strlen(playPauseIcon) + strlen(nextLabel) + 2 * spacing;
    int pbStartX = (width - pbWidth) / 2; if (pbStartX < 1) pbStartX = 1;
    int volWidth = strlen(volDownLabel) + strlen(volUpLabel) + spacing;
    int volStartX = width - volWidth - 2;

    // Calculate and store coordinates for handleMouse
    int currentX = pbStartX;
    prevX_start = currentX; prevX_end = prevX_start + strlen(prevLabel);
    mvwprintw(win, 3, currentX, "%s", prevLabel);

    currentX += strlen(prevLabel) + spacing;
    playPauseX_start = currentX; playPauseX_end = playPauseX_start + strlen(playPauseIcon);
    mvwprintw(win, 3, currentX, "%s", playPauseIcon);

    currentX += strlen(playPauseIcon) + spacing;
    nextX_start = currentX; nextX_end = nextX_start + strlen(nextLabel);
    mvwprintw(win, 3, currentX, "%s", nextLabel);

    currentX = volStartX;
    volDownX_start = currentX; volDownX_end = volDownX_start + strlen(volDownLabel);
    mvwprintw(win, 3, currentX, "%s", volDownLabel);

    currentX += strlen(volDownLabel) + spacing;
    volUpX_start = currentX; volUpX_end = volUpX_start + strlen(volUpLabel);
    mvwprintw(win, 3, currentX, "%s", volUpLabel);
    // --- END Controls ---

    // Focus indicator
    if (hasFocus) mvwprintw(win, 1, 1, ">");

    wnoutrefresh(win); // Update virtual screen
}

BottomBarAction BottomBarView::handleMouse(int localY, int localX) {
    if (localY != 3) return BottomBarAction::NONE; // Only handle clicks on the control line (Y=3)

    if (localX >= prevX_start && localX < prevX_end) return BottomBarAction::PREV_TRACK;
    if (localX >= playPauseX_start && localX < playPauseX_end) return BottomBarAction::TOGGLE_PLAY_PAUSE;
    if (localX >= nextX_start && localX < nextX_end) return BottomBarAction::NEXT_TRACK;
    if (localX >= volDownX_start && localX < volDownX_end) return BottomBarAction::VOLUME_DOWN;
    if (localX >= volUpX_start && localX < volUpX_end) return BottomBarAction::VOLUME_UP;

    return BottomBarAction::NONE; // Click didn't hit a known button area
}

// --- NEW KEYBOARD HANDLING ---
BottomBarAction BottomBarView::handleKeyboard(int key) {
     switch (key) {
         case ' ': // Space or P for Play/Pause
         case 'p':
             return BottomBarAction::TOGGLE_PLAY_PAUSE;
         case KEY_LEFT: // Left arrow for Previous
         case '<':
             return BottomBarAction::PREV_TRACK;
         case KEY_RIGHT: // Right arrow for Next
         case '>':
             return BottomBarAction::NEXT_TRACK;
         case '-': // Minus for Volume Down
             return BottomBarAction::VOLUME_DOWN;
         case '+': // Plus (often Shift+=) for Volume Up
         case '=': // Equals key often used too
             return BottomBarAction::VOLUME_UP;
         default:
             return BottomBarAction::NONE;
     }
}
