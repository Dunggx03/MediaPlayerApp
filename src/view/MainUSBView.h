#pragma once
#include "view/IMainAreaView.h"
#include "utils/NcursesUI.h"
#include "model/MediaManager.h"
#include "model/MediaFile.h"

class AppController;

class MainUSBView : public IMainAreaView {
public:
    MainUSBView(NcursesUI* ui, WINDOW* win, AppController* controller);
    void draw(FocusArea focus) override;
    MainAreaAction handleInput(InputEvent event, FocusArea focus) override;
    MainAreaAction handleMouse(int localY, int localX) override;
    MediaFile* getSelectedFile() const;

private:
    NcursesUI* ui;
    WINDOW* win;
    AppController* appController;
    MediaManager* mediaManager;

    bool usbConnected;
    bool fileExplicitlySelected;

    int filePage, totalPages, itemsPerPage, fileSelected;

    // --- Button coordinates ---
    int reloadBtnY, reloadBtnX, reloadBtnW;
    int ejectBtnY, ejectBtnX, ejectBtnW;
    int editBtnY, editBtnX, editBtnW;
    int addBtnY, addBtnX, addBtnW;
    int prevBtnY, prevBtnX, prevBtnW;
    int nextBtnY, nextBtnX, nextBtnW;

    void updateUSBStatus();
    
};
