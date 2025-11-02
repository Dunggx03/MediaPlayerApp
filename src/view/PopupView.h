#pragma once
#include "utils/NcursesUI.h"
#include <string>
#include <vector>
#include <optional>
#include <map>

class Metadata; 

class PopupView {
public:
    PopupView(NcursesUI* ui, int parentHeight, int parentWidth);
    ~PopupView();

    std::optional<std::string> showTextInput(const std::string& prompt, const std::string& initialValue = "");
    std::optional<int> showListSelection(const std::string& title, const std::vector<std::string>& options);

    bool showMetadataEditor(Metadata* metadata);

private:
    void drawWindow(const std::string& title);
    void clearWindow();
    std::string getLineInput(int y, int x, const std::string& initialValue, int maxLength);

    NcursesUI* ui_;
    WINDOW* win_;
    int winHeight_, winWidth_;
    int parentH_, parentW_;
};