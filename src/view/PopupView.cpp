#include "view/PopupView.h"
#include "model/Metadata.h"
#include <string.h>
#include <algorithm>
#include <iostream> 

PopupView::PopupView(NcursesUI* ui, int parentHeight, int parentWidth)
    : ui_(ui), win_(nullptr), parentH_(parentHeight), parentW_(parentWidth)
{
    winHeight_ = std::min(10, parentH_ - 4);
    winWidth_ = std::min(50, parentW_ - 10);
    if (winHeight_ < 5) winHeight_ = 5;
    if (winWidth_ < 20) winWidth_ = 20;
}

PopupView::~PopupView() {
    clearWindow();
}

void PopupView::drawWindow(const std::string& title) {
    if (win_) { 
        delwin(win_);
    }
    int y = (parentH_ - winHeight_) / 2;
    int x = (parentW_ - winWidth_) / 2;
    
    win_ = ui_->drawWindow(y, x, winHeight_, winWidth_, title); 
    keypad(win_, TRUE);
}

void PopupView::clearWindow() {
    if (win_) { 
        delwin(win_);
        win_ = nullptr;
    }
}

std::optional<std::string> PopupView::showTextInput(const std::string& prompt, const std::string& initialValue) {
    drawWindow("Input Required");
    curs_set(1);
    echo();

    mvwprintw(win_, 1, 2, "%.*s", winWidth_ - 4, prompt.c_str()); 

    WINDOW* inputWin = derwin(win_, 1, winWidth_ - 4, 3, 2); 
    wattron(inputWin, A_REVERSE);
    mvwprintw(inputWin, 0, 0, "%-*s", winWidth_ - 4, "");
    wattroff(inputWin, A_REVERSE);

    char buffer[256];
    strncpy(buffer, initialValue.c_str(), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    wmove(inputWin, 0, 0);
    wprintw(inputWin, "%s", buffer);
    wrefresh(inputWin);

    int result = wgetnstr(inputWin, buffer, sizeof(buffer) - 1);

    delwin(inputWin);
    noecho();
    curs_set(0);
    clearWindow();

    if (result == ERR) {
         return std::nullopt;
    } else {
        return std::string(buffer);
    }
}


std::optional<int> PopupView::showListSelection(const std::string& title, const std::vector<std::string>& options) {
    if (options.empty()) return std::nullopt;

    drawWindow(title);
    int selected = 0;
    int scrollOffset = 0;
    int maxDisplay = winHeight_ - 4;

    while (true) {
        werase(win_);
        box(win_, 0, 0);
        mvwprintw(win_, 0, (winWidth_ - title.length()) / 2, "%.*s", winWidth_ - 2, title.c_str());
        mvwprintw(win_, 1, 2, "Use UP/DOWN/ENTER. ESC to cancel.");

        for (int i = 0; i < maxDisplay; ++i) {
            int currentOptionIndex = scrollOffset + i;
            if (currentOptionIndex >= (int)options.size()) break;

            if (currentOptionIndex == selected) {
                wattron(win_, A_REVERSE);
            }
            mvwprintw(win_, i + 2, 2, "%.*s", winWidth_ - 4, options[currentOptionIndex].c_str());
            wattroff(win_, A_REVERSE);
        }
        wrefresh(win_); 
        int ch = wgetch(win_);                      
        switch (ch) {
            case KEY_UP:
                selected = std::max(0, selected - 1);
                if (selected < scrollOffset) scrollOffset = selected;
                break;
            case KEY_DOWN:
                selected = std::min((int)options.size() - 1, selected + 1);
                if (selected >= scrollOffset + maxDisplay) scrollOffset = selected - maxDisplay + 1;
                break;
            case 10: // Enter
                clearWindow();
                return selected;
            case 27: // ESC
            case 'q':
                clearWindow();
                return std::nullopt;
        }
    }
}

std::string PopupView::getLineInput(int y, int x, const std::string& initialValue, int maxLength) {
    curs_set(1);
    echo();
    
    char buffer[maxLength + 1];
    strncpy(buffer, initialValue.c_str(), maxLength);
    buffer[maxLength] = '\0';

    wattron(win_, A_REVERSE);
    mvwprintw(win_, y, x, "%-*s", maxLength, buffer);
    wattroff(win_, A_REVERSE);
    
    wmove(win_, y, x);
    
    int result = wgetnstr(win_, buffer, maxLength);
    
    noecho();
    curs_set(0);

    if (result == ERR) {
        return initialValue;
    }
    return std::string(buffer);
}

bool PopupView::showMetadataEditor(Metadata* metadata) {
    if (!metadata) return false;

    winHeight_ = 12;
    winWidth_ = 60;
    drawWindow("Edit Metadata");

    std::vector<std::pair<std::string, std::string>> fields;
    fields.push_back({"Title", metadata->title});
    fields.push_back({"Artist", metadata->getField("artist")});
    fields.push_back({"Album", metadata->getField("album")});
    fields.push_back({"Genre", metadata->getField("genre")});
    fields.push_back({"Year", metadata->getField("year")});

    int selectedField = 0;
    bool saved = false;

    while (true) {
        werase(win_);
        box(win_, 0, 0);
        mvwprintw(win_, 0, 2, "Edit Metadata");
        mvwprintw(win_, 1, 2, "UP/DOWN: Navigate | ENTER: Edit | 's': Save | 'q': Cancel");

        int y = 3;
        for (size_t i = 0; i < fields.size(); ++i) {
            std::string label = fields[i].first + ":";
            std::string value = fields[i].second;
            
            mvwprintw(win_, y + i, 2, "%-10s", label.c_str());
            
            if ((int)i == selectedField) wattron(win_, A_REVERSE | A_BOLD);
            mvwprintw(win_, y + i, 12, " %.*s ", winWidth_ - 15, value.c_str());
            if ((int)i == selectedField) wattroff(win_, A_REVERSE | A_BOLD);
        }
        
        mvwprintw(win_, y + fields.size() + 2, 2, "[S]ave & Close");
        mvwprintw(win_, y + fields.size() + 2, 20, "[Q]uit");

        wrefresh(win_);
        
        int ch = wgetch(win_);
        switch (ch) {
            case KEY_UP:
                selectedField = (selectedField - 1 + fields.size()) % fields.size();
                break;
            case KEY_DOWN:
                selectedField = (selectedField + 1) % fields.size();
                break;
            case 10: // Enter
            {
                std::string newValue = getLineInput(
                    y + selectedField, 12,
                    fields[selectedField].second,
                    winWidth_ - 15
                );
                fields[selectedField].second = newValue;
                break;
            }
            case 's':
            case 'S':
            {
                for (const auto& field : fields) {
                    std::string key = field.first;
                    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                    metadata->setField(key, field.second);
                }
                saved = true;
                goto exit_loop;
            }
            case 'q':
            case 'Q':
            case 27: // ESC
                saved = false;
                goto exit_loop;
        }
    }

exit_loop:
    clearWindow();
    return saved;
}