#include "NcursesUI.h"
#include <iostream> 

NcursesUI::NcursesUI() : isInitialized(false), screenHeight(0), screenWidth(0) {}

NcursesUI::~NcursesUI() {
    if (isInitialized) {
        closeScreen();
    }
}

bool NcursesUI::initScreen() {
    if (isInitialized) return true;
    if (initscr() == NULL) return false;

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    timeout(100); // Wait 100ms for input, then return ERR if none

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        wbkgd(stdscr, COLOR_PAIR(1));
    }
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    getScreenDimensions(screenHeight, screenWidth);
    isInitialized = true;
    return true;
}

void NcursesUI::closeScreen() {
    if (isInitialized) {
        endwin();
        isInitialized = false;
    }
}

WINDOW* NcursesUI::drawWindow(int y, int x, int height, int width, const std::string& title) {
    WINDOW* win = newwin(height, width, y, x);
    box(win, 0, 0);
    if (!title.empty() && width > 4) { // Ensure width is enough for title
        mvwprintw(win, 0, (width - title.length()) / 2, "%.*s", width - 2, title.c_str()); // Limit title width
    }
    wrefresh(win);
    return win;
}

void NcursesUI::printText(WINDOW* win, int y, int x, const std::string& text) {
    if (win == NULL) win = stdscr;
    // Get window dimensions to prevent writing outside bounds
    int max_y = getmaxy(win);
    int max_x = getmaxx(win);
    // Basic bounds check (adjust based on border presence if needed)
    if (y > 0 && y < max_y -1 && x > 0 && x < max_x - 1) {
         mvwprintw(win, y, x, "%.*s", max_x - x -1 , text.c_str()); // Limit text width
    }
}

InputEvent NcursesUI::getInput() {
    InputEvent event;
    // wgetch will now return ERR after 100ms if no input
    int ch = wgetch(stdscr);

    if (ch == ERR) {
        // No input received within the timeout period
        event.type = InputEvent::UNKNOWN; // Or a new type like TIMEOUT
    }
    else if (ch == KEY_MOUSE) {
        MEVENT mouseEvent;
        if (getmouse(&mouseEvent) == OK) {
            event.type = InputEvent::MOUSE;
            event.mouseX = mouseEvent.x;
            event.mouseY = mouseEvent.y;
            // You might want to capture mouseEvent.bstate here too
        } else {
             event.type = InputEvent::UNKNOWN; // Failed to get mouse event details
        }
    } else {
        event.type = InputEvent::KEYBOARD;
        event.key = ch;
    }

    return event;
}

void NcursesUI::refreshScreen() {
    refresh();
}

void NcursesUI::clearScreen() {
    clear();
}

void NcursesUI::getScreenDimensions(int& outHeight, int& outWidth) {
    getmaxyx(stdscr, outHeight, outWidth);
    this->screenHeight = outHeight;
    this->screenWidth = outWidth;
}