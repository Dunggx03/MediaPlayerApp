#pragma once
#include <ncurses.h>
#include <string>

struct InputEvent {
    enum EventType { KEYBOARD, MOUSE, UNKNOWN };
    
    EventType type = UNKNOWN;
    int key = 0;      
    int mouseX = 0;    
    int mouseY = 0;    
};

class NcursesUI {
public:
    NcursesUI();

    ~NcursesUI();

    bool initScreen();

    void closeScreen();

    WINDOW* drawWindow(int y, int x, int height, int width, const std::string& title = "");
    void printText(WINDOW* win, int y, int x, const std::string& text);

    InputEvent getInput();

    void refreshScreen();

    void clearScreen();
    void getScreenDimensions(int& outHeight, int& outWidth);

private:
    bool isInitialized;
    int screenHeight;
    int screenWidth;
};