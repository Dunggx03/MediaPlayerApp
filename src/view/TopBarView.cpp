#include "view/TopBarView.h"
#include <cstring>

TopBarView::TopBarView(NcursesUI* ui, WINDOW* win, const std::string& title)
    : ui(ui), win(win), title(title) {}

void TopBarView::draw() {
    werase(win);
    box(win, 0, 0);

    int width = getmaxx(win);
    wattron(win, A_BOLD | A_REVERSE);
    mvwprintw(win, 1, (width - title.size()) / 2, "%s", title.c_str());
    wattroff(win, A_BOLD | A_REVERSE);

    wnoutrefresh(win);
}

