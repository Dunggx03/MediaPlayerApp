#pragma once
#include "utils/NcursesUI.h"
#include <vector>
#include <string>

enum class AppMode;
enum class FocusArea;

class SidebarView {
public:
    SidebarView(NcursesUI* ui, WINDOW* win);
    void draw(bool hasFocus);
    AppMode handleInput(InputEvent event);
    AppMode handleMouse(int localY, int localX);
    bool shouldExit() const;

private:
    AppMode getCurrentModeSelection() const;

    NcursesUI* ui;
    WINDOW* win;
    int selectedOption;
    bool exitFlag;
    std::vector<std::string> options;
};