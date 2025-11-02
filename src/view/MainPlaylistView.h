#pragma once
#include "view/IMainAreaView.h"
#include <vector>
#include <string>
#include "model/PlaylistManager.h"
#include "model/Playlist.h"
#include "model/MediaFile.h"

class MainPlaylistView : public IMainAreaView {
public:
    MainPlaylistView(NcursesUI* ui, WINDOW* win, PlaylistManager* manager);
    void draw(FocusArea focus) override;
    
    MainAreaAction handleInput(InputEvent event, FocusArea focus) override;
    MainAreaAction handleMouse(int localY, int localX) override;
    MediaFile* getSelectedTrack() const;

    Playlist* getSelectedPlaylist() const;

    int getSelectedPlaylistIndex() const; 
    int getSelectedTrackIndex() const;
private:
    NcursesUI* ui;
    WINDOW* win;
    PlaylistManager* playlistManager;
    int playlistSelected;
    int trackSelected;

    // Store button locations
    int createBtnY, createBtnX, createBtnW;
    int deleteBtnY, deleteBtnX, deleteBtnW;
    int playBtnY, playBtnX, playBtnW;
    int removeBtnY, removeBtnX, removeBtnW;

    // --- Pagination for Playlists ---
    int playlistPage;
    int totalPlaylistPages;
    int playlistsPerPage;

    // --- Button Coordinates ---
    int prevBtnY, prevBtnX, prevBtnW;
    int nextBtnY, nextBtnX, nextBtnW;
};