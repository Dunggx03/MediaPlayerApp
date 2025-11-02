#pragma once
#include "view/IMainAreaView.h"
#include <vector>
#include <string>
#include "model/MediaManager.h"
#include "model/MediaFile.h"

class MainFileView : public IMainAreaView {
public:
    MainFileView(NcursesUI* ui, WINDOW* win, MediaManager* manager);
    void draw(FocusArea focus) override;
    MainAreaAction handleInput(InputEvent event, FocusArea focus) override;
    MainAreaAction handleMouse(int localY, int localX) override;
    MediaFile* getSelectedFile() const;

private:
    NcursesUI* ui;
    WINDOW* win;
    MediaManager* mediaManager;
    int filePage, totalPages, itemsPerPage, fileSelected;

    //Store button locations for mouse clicks
    int editButtonY, editButtonX, editButtonW;
    int addButtonY, addButtonX, addButtonW;
    int prevBtnY, prevBtnX, prevBtnW;
    int nextBtnY, nextBtnX, nextBtnW;

    bool fileExplicitlySelected;
};