#include "view/SidebarView.h"
#include "view/UIManager.h"

SidebarView::SidebarView(NcursesUI* ui, WINDOW* win)
    : ui(ui), win(win), selectedOption(0), exitFlag(false)
{
    options = {"FILE", "PLAYLIST", "USB", "EXIT"};
}

void SidebarView::draw(bool hasFocus) {
    werase(win);
    box(win, 0, 0);

    for (size_t i = 0; i < options.size(); ++i) {
        if (static_cast<int>(i) == selectedOption) {
            wattron(win, A_REVERSE);
            if (hasFocus) wattron(win, A_BOLD);
        }

        mvwprintw(win, 3 + i, 3, "%s", options[i].c_str());
        wattroff(win, A_REVERSE | A_BOLD);
    }
    wnoutrefresh(win);
}

AppMode SidebarView::getCurrentModeSelection() const {
     if (selectedOption == 1) return AppMode::PLAYLISTS;
     if (selectedOption == 2) return AppMode::USB_BROWSER;
     return AppMode::FILE_BROWSER; // Default
}


AppMode SidebarView::handleInput(InputEvent event) {
     if (event.type != InputEvent::KEYBOARD) return getCurrentModeSelection(); // Return current if not keyboard

     switch(event.key) {
        case KEY_UP: selectedOption = (selectedOption - 1 + options.size()) % options.size(); break;
        case KEY_DOWN: selectedOption = (selectedOption + 1) % options.size(); break;
        case 10: // Enter
            if (selectedOption == 3) exitFlag = true;
            break;
     }
     return getCurrentModeSelection(); // Return the new mode selected
}

AppMode SidebarView::handleMouse(int localY, int localX) {
    int clickedOption = localY - 3; 
    if (clickedOption >= 0 && static_cast<size_t>(clickedOption) < options.size()) {
        selectedOption = clickedOption;
        if (selectedOption == 3) exitFlag = true;
    }
    return getCurrentModeSelection(); 
}


bool SidebarView::shouldExit() const {
    return exitFlag;
}