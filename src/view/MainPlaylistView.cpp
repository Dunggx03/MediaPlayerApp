#include "view/MainPlaylistView.h"
#include "model/Playlist.h"
#include "model/MediaFile.h"
#include <algorithm>
#include <string>
#include <tuple>
#include <vector>
#include <cmath> 

MainPlaylistView::MainPlaylistView(NcursesUI* ui, WINDOW* win, PlaylistManager* manager)
    : ui(ui), win(win), playlistManager(manager),
      playlistSelected(0), trackSelected(0),
      playlistPage(1) // Start playlist page at 1
{
    // Initialize button positions
    createBtnY = createBtnX = createBtnW = 0;
    deleteBtnY = deleteBtnX = deleteBtnW = 0;
    playBtnY = playBtnX = playBtnW = 0;
    removeBtnY = removeBtnX = removeBtnW = 0;
    prevBtnY = prevBtnX = prevBtnW = 0;
    nextBtnY = nextBtnX = nextBtnW = 0;

    // Calculate initial pagination for playlists
    int width, height;
    getmaxyx(win, height, width);
    // Lines available for playlist names (exclude box, header, buttons)
    int availableLines = height - 4 - 2 - 1; // -1 for button row
    if (availableLines < 1) availableLines = 1;
    playlistsPerPage = std::min(25, availableLines); // Apply max 25 limit

    totalPlaylistPages = 0;
}

void MainPlaylistView::draw(FocusArea focus) {
    werase(win);
    box(win, 0, 0);

    int width, height;
    getmaxyx(win, height, width);
    int listWidth = width / 2;
    int detailWidth = width - listWidth;

    if (!playlistManager) { // Safety check
        mvwprintw(win, 2, 3, "Error: PlaylistManager is null!");
        wnoutrefresh(win);
        return;
    }

    std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();
    int totalPlaylists = playlists.size();

    int availableLines = height - 4 - 2 - 1; // Recalculate based on current height
    if (availableLines < 1) availableLines = 1;
    playlistsPerPage = std::min(25, availableLines);
    totalPlaylistPages = (totalPlaylists > 0) ? static_cast<int>(std::ceil(static_cast<double>(totalPlaylists) / playlistsPerPage)) : 1;
    if (playlistPage > totalPlaylistPages) playlistPage = totalPlaylistPages;

    std::string pageInfo = "Page " + std::to_string(playlistPage) + "/" + std::to_string(totalPlaylistPages);
    std::string prevLabel = "[< Prev]"; prevBtnW = prevLabel.length();
    std::string nextLabel = "[Next >]"; nextBtnW = nextLabel.length();
    prevBtnY = 2; prevBtnX = 3; // Position Prev left
    nextBtnY = 2; nextBtnX = listWidth - nextBtnW - 2; // Position Next right
    mvwprintw(win, prevBtnY, prevBtnX, "%s", prevLabel.c_str());
    mvwprintw(win, 2, (listWidth - pageInfo.length()) / 2, "%s", pageInfo.c_str()); // Center page info
    mvwprintw(win, nextBtnY, nextBtnX, "%s", nextLabel.c_str());

    int start = (playlistPage - 1) * playlistsPerPage;
    for (int i = 0; i < playlistsPerPage; ++i) { // Loop up to playlistsPerPage
        int lineY = 4 + i;
        if (lineY >= height - 3) break; // Stop before button row

        int playlistIdxGlobal = start + i;

        if (playlistIdxGlobal < totalPlaylists) { // Check if index is valid
            if (focus == FocusArea::MAIN_LIST && playlistIdxGlobal == playlistSelected) {
                 wattron(win, A_REVERSE | A_BOLD);
            }
            std::string name = playlists[playlistIdxGlobal]->getName();
            std::string count = "(" + std::to_string(playlists[playlistIdxGlobal]->getTracks().size()) + ")";
            std::string entry = name + " " + count;
            mvwprintw(win, lineY, 3, "%.*s", listWidth - 5, entry.c_str()); // Truncate display
            wattroff(win, A_REVERSE | A_BOLD);
        } else {
             mvwprintw(win, lineY, 3, "%*s", listWidth - 5, ""); // Clear unused lines
        }
    }

    // --- Draw track list panel ---
    // Header
    mvwprintw(win, 2, listWidth + (detailWidth - 18)/2, "Tracks in Playlist"); // Center title

    // --- ADD BORDER ---
    mvwhline(win, 3, listWidth + 1, ACS_HLINE, detailWidth - 2); // Top border
    mvwhline(win, height - 4, listWidth + 1, ACS_HLINE, detailWidth - 2); // Bottom border
    mvwvline(win, 4, listWidth, ACS_VLINE, height - 7); // Left border
    mvwvline(win, 4, width - 2, ACS_VLINE, height - 7); // Right border
    mvwaddch(win, 3, listWidth, ACS_ULCORNER);
    mvwaddch(win, 3, width - 2, ACS_URCORNER);
    mvwaddch(win, height - 4, listWidth, ACS_LLCORNER);
    mvwaddch(win, height - 4, width - 2, ACS_LRCORNER);
    // --- END BORDER ---

    // Track content
    std::vector<MediaFile*> tracks;
    Playlist* currentSelectedPlaylist = getSelectedPlaylist(); // Use helper
    int maxTracksToShow = height - 4 - 2 - 1; // Lines inside border, excluding button
    if (maxTracksToShow < 0) maxTracksToShow = 0;

    if (currentSelectedPlaylist) {
        tracks = currentSelectedPlaylist->getTracks();
        if (tracks.empty()) {
            mvwprintw(win, 4, listWidth + 2, "(No tracks)");
        } else {
            for (size_t i = 0; i < tracks.size() && (int)i < maxTracksToShow; ++i) {
                if (4 + (int)i >= height - 4) break; // Don't draw outside border
                if (focus == FocusArea::MAIN_DETAIL && (int)i == trackSelected) {
                    wattron(win, A_REVERSE | A_BOLD);
                }
                mvwprintw(win, 4 + i, listWidth + 2, "%.*s", detailWidth - 4, tracks[i]->getFileName().c_str());
                wattroff(win, A_REVERSE | A_BOLD);
            }
        }
    } else if (!playlists.empty()) {
        mvwprintw(win, 4, listWidth + 2, "(Select a playlist)");
    } else {
        mvwprintw(win, 4, listWidth + 2, "(No playlists created)");
    }

    // --- Draw Buttons (at bottom, coordinates stored for mouse handling) ---
    int buttonY = height - 3; // All buttons on the same line

    // Playlist buttons (Left panel)
    std::string createLabel = "[Create]"; createBtnW = createLabel.length();
    std::string deleteLabel = "[Delete]"; deleteBtnW = deleteLabel.length();
    std::string playLabel = "[Play]"; playBtnW = playLabel.length();
    
    // Align left, center, and right within the left panel
    createBtnY = buttonY; createBtnX = 3; // Align left
    deleteBtnY = buttonY; deleteBtnX = (listWidth - deleteBtnW) / 2; // Align center
    playBtnY = buttonY; playBtnX = listWidth - playBtnW - 2; // Align right
    
    mvwprintw(win, createBtnY, createBtnX, "%s", createLabel.c_str());
    mvwprintw(win, deleteBtnY, deleteBtnX, "%s", deleteLabel.c_str());
    mvwprintw(win, playBtnY, playBtnX, "%s", playLabel.c_str());

    // Track button (Right panel) - Center it
    std::string removeLabel = "[Remove Song]"; removeBtnW = removeLabel.length();
    removeBtnY = buttonY;
    // Calculate center position relative to the start of the right panel
    removeBtnX = listWidth + (detailWidth - removeBtnW) / 2;
    
    mvwprintw(win, removeBtnY, removeBtnX, "%s", removeLabel.c_str());

    // Optional highlight based on focus
    if (focus == FocusArea::MAIN_LIST) { wattron(win, A_BOLD); mvwprintw(win, createBtnY, createBtnX, "%s", createLabel.c_str()); wattroff(win, A_BOLD); }
    if (focus == FocusArea::MAIN_DETAIL && !tracks.empty()) { wattron(win, A_BOLD); mvwprintw(win, removeBtnY, removeBtnX, "%s", removeLabel.c_str()); wattroff(win, A_BOLD); }

    wnoutrefresh(win);
}

MainAreaAction MainPlaylistView::handleInput(InputEvent event, FocusArea focus) {
     if (!playlistManager) return MainAreaAction::NONE;

    std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();
    int playlistCount = playlists.size();
    int trackCount = 0;
    Playlist* currentPlaylist = getSelectedPlaylist();
    if (currentPlaylist) trackCount = currentPlaylist->getTracks().size();

    if (focus == FocusArea::MAIN_LIST) {
        int oldSelected = playlistSelected;
        if (event.key == KEY_DOWN && playlistCount > 0) playlistSelected = std::min(playlistSelected + 1, playlistCount - 1);
        if (event.key == KEY_UP) playlistSelected = std::max(0, playlistSelected - 1);
        // --- ADD PAGINATION KEYS ---
        else if (event.key == KEY_PPAGE) { // Page Up
            if (playlistPage > 1) {
                playlistPage--;
                playlistSelected = (playlistPage - 1) * playlistsPerPage; // Select top
            }
        }
        else if (event.key == KEY_NPAGE) { // Page Down
            if (playlistPage < totalPlaylistPages) {
                playlistPage++;
                playlistSelected = (playlistPage - 1) * playlistsPerPage; // Select top
                playlistSelected = std::min(playlistSelected, playlistCount - 1); // Clamp
            }
        }
        // --- END PAGINATION KEYS ---

        // Auto-scroll page if selection moved via UP/DOWN
        if (playlistSelected != oldSelected) {
            trackSelected = 0; // Reset track selection
            int newPage = (playlistSelected < 0) ? 1 : (playlistSelected / playlistsPerPage) + 1;
            if (newPage != playlistPage) playlistPage = newPage;
        }

        if (event.key == 'c') return MainAreaAction::CREATE_PLAYLIST;

    } else if (focus == FocusArea::MAIN_DETAIL) {
        if (trackCount > 0) {
            if (event.key == KEY_DOWN) trackSelected = std::min(trackSelected + 1, trackCount - 1);
            if (event.key == KEY_UP) trackSelected = std::max(0, trackSelected - 1);
        }
        // Button Activation
        if (event.key == 10) { // Enter - Activate [Remove Song]
            if (trackCount > 0 && trackSelected >= 0) return MainAreaAction::REMOVE_TRACK_FROM_PLAYLIST;
        }
    }

    return MainAreaAction::NONE;
}

MainAreaAction MainPlaylistView::handleMouse(int localY, int localX) {
     if (!playlistManager) return MainAreaAction::NONE;

    int width; getmaxyx(win, std::ignore, width); int listWidth = width / 2;
    int listStartY = 4;
    int clickedIndexOnPage = localY - listStartY; // Index relative to page start

    // --- Check Header Pagination Buttons ---
    if (localY == prevBtnY) {
        if (localX >= prevBtnX && localX < prevBtnX + prevBtnW && playlistPage > 1) {
            playlistPage--;
            playlistSelected = (playlistPage - 1) * playlistsPerPage;
            trackSelected = 0;
            return MainAreaAction::NONE;
        }
        if (localX >= nextBtnX && localX < nextBtnX + nextBtnW && playlistPage < totalPlaylistPages) {
            playlistPage++;
            playlistSelected = (playlistPage - 1) * playlistsPerPage;
            int playlistCount = playlistManager->getAllPlaylists().size();
            playlistSelected = std::min(playlistSelected, playlistCount - 1);
            trackSelected = 0;
            return MainAreaAction::NONE;
        }
    }

    // --- Check Bottom Buttons ---
    if (localY == createBtnY) { // Bottom button row
        if (localX >= createBtnX && localX < createBtnX + createBtnW) return MainAreaAction::CREATE_PLAYLIST;
        if (localX >= deleteBtnX && localX < deleteBtnX + deleteBtnW) return MainAreaAction::DELETE_PLAYLIST;
        if (localX >= playBtnX && localX < playBtnX + playBtnW) return MainAreaAction::PLAY_PLAYLIST;
    }
    if (localY == removeBtnY) { // Bottom button row (can be same Y)
         if (localX >= removeBtnX && localX < removeBtnX + removeBtnW) return MainAreaAction::REMOVE_TRACK_FROM_PLAYLIST;
    }


    // --- Handle List Clicks ---
    std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();
    int totalPlaylists = playlists.size();
    if (totalPlaylists == 0 && localY >= listStartY) return MainAreaAction::NONE; // No lists to click

    if (localX < listWidth) { // Clicked on playlist list
        int clickedIndexGlobal = (playlistPage - 1) * playlistsPerPage + clickedIndexOnPage;
        if (clickedIndexOnPage >= 0 && clickedIndexGlobal < totalPlaylists) {
            if (playlistSelected != clickedIndexGlobal) {
                 playlistSelected = clickedIndexGlobal;
                 trackSelected = 0;
            }
        }
    } else { // Clicked on track list
        Playlist* currentPlaylist = getSelectedPlaylist();
        if (currentPlaylist) {
            int trackCount = currentPlaylist->getTracks().size();
            if (clickedIndexOnPage >= 0 && clickedIndexOnPage < trackCount) {
                trackSelected = clickedIndexOnPage;
            }
        }
    }
    return MainAreaAction::NONE; // Click was on list item, not button
}


MediaFile* MainPlaylistView::getSelectedTrack() const {
    Playlist* selectedPlaylist = getSelectedPlaylist(); // Use helper
    if (!selectedPlaylist) return nullptr;

    const std::vector<MediaFile*>& tracks = selectedPlaylist->getTracks();
    if (trackSelected < 0 || static_cast<size_t>(trackSelected) >= tracks.size()) return nullptr;
    return tracks[trackSelected];
}

Playlist* MainPlaylistView::getSelectedPlaylist() const {
    if (!playlistManager) return nullptr;
    std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();
    // Use global index directly
    if (playlistSelected >= 0 && static_cast<size_t>(playlistSelected) < playlists.size()) {
        return playlists[playlistSelected];
    }
    return nullptr;
}

int MainPlaylistView::getSelectedPlaylistIndex() const {
     if (!playlistManager) return -1;
     std::vector<Playlist*> playlists = playlistManager->getAllPlaylists();
     // Use global index directly
     if (playlistSelected >= 0 && static_cast<size_t>(playlistSelected) < playlists.size()) {
        return playlistSelected;
     }
     return -1;
}

int MainPlaylistView::getSelectedTrackIndex() const {
    //return index if a playlist is selected
    Playlist* pl = getSelectedPlaylist();
    if (pl && trackSelected >= 0 && (size_t)trackSelected < pl->getTracks().size()) {
        return trackSelected;
    }
    return 0; // return 0 (first song) if nothing selected
}