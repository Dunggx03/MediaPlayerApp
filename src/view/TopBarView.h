#pragma once
#include "utils/NcursesUI.h"
#include <string>

class TopBarView {
public:
    TopBarView(NcursesUI* ui, WINDOW* win, const std::string& title);
    void draw();

private:
    NcursesUI* ui;
    WINDOW* win;
    std::string title;
};
