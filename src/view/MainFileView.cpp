#include "view/MainFileView.h"
#include "model/MediaFile.h"
#include "model/Metadata.h"
#include <cmath>
#include <algorithm>
#include <tuple>
#include <iostream> // For debug if needed

// --- UPDATED CONSTRUCTOR ---
MainFileView::MainFileView(NcursesUI* ui, WINDOW* win, MediaManager* manager)
    : ui(ui), win(win), mediaManager(manager),
      filePage(1),
      fileSelected(-1), // Start with no file selected
      fileExplicitlySelected(false) // Track user selection
{
    // Initialize button positions to safe defaults
    prevBtnY = prevBtnX = prevBtnW = 0;
    nextBtnY = nextBtnX = nextBtnW = 0;
    editButtonY = editButtonX = editButtonW = 0;
    addButtonY = addButtonX = addButtonW = 0;

    // Calculate initial pagination (will be recalculated in draw)
    int width, height;
    getmaxyx(win, height, width);
    // Calculate max lines available for files (excluding borders, header, footer/buttons)
    int availableLines = height - 4 - 2; // -4 box, -2 header/footer line
    if(availableLines < 1) availableLines = 1;
    // Limit items per page to 25 or available lines, whichever is smaller
    itemsPerPage = std::min(25, availableLines);

    totalPages = mediaManager ? mediaManager->getTotalPages(itemsPerPage) : 1;
    if (totalPages == 0) totalPages = 1;
}
// --- END CONSTRUCTOR ---

// --- UPDATED DRAW FUNCTION ---
void MainFileView::draw(FocusArea focus) {
    werase(win); // Use werase instead of wclear for potentially less flicker
    box(win, 0, 0);

    int width, height;
    getmaxyx(win, height, width);
    int listWidth = width / 2;
    int detailWidth = width - listWidth;

    // Recalculate pagination based on current size
    int availableLines = height - 4 - 2;
    if(availableLines < 1) availableLines = 1;
    itemsPerPage = std::min(25, availableLines); // Apply 25 limit
    totalPages = mediaManager ? mediaManager->getTotalPages(itemsPerPage) : 1;
    if (totalPages == 0) totalPages = 1;
    if (filePage > totalPages) filePage = totalPages;
    //int totalFiles = mediaManager ? mediaManager->getTotalFileCount() : 0;

    // --- Draw list panel ---
    // Header with buttons
    std::string pageInfo = "Page " + std::to_string(filePage) + "/" + std::to_string(totalPages);
    std::string prevLabel = "[< Prev]"; prevBtnW = prevLabel.length();
    std::string nextLabel = "[Next >]"; nextBtnW = nextLabel.length();
    prevBtnY = 2; prevBtnX = 3; // Position Prev button left
    nextBtnY = 2; nextBtnX = listWidth - nextBtnW - 2; // Position Next button right
    int titleX = (listWidth - pageInfo.length()) / 2; // Center page info

    mvwprintw(win, prevBtnY, prevBtnX, "%s", prevLabel.c_str());
    mvwprintw(win, 2, titleX, "%s", pageInfo.c_str());
    mvwprintw(win, nextBtnY, nextBtnX, "%s", nextLabel.c_str());

    // File list content (loop up to itemsPerPage)
    std::vector<MediaFile*> filesOnPage = mediaManager ? mediaManager->getPage(filePage, itemsPerPage) : std::vector<MediaFile*>();
    for (size_t i = 0; i < filesOnPage.size(); ++i) {
        int lineY = 4 + i;
        if (lineY >= height - 2) break; // Don't draw outside window

        // The global index of the current file being drawn
        int fileIdxGlobal = (filePage - 1) * itemsPerPage + i;

        // Highlight if this file is the currently selected one
        if (focus == FocusArea::MAIN_LIST && fileIdxGlobal == fileSelected) {
            wattron(win, A_REVERSE | A_BOLD);
        }

        // Draw the filename from the filesOnPage vector using index 'i'
        mvwprintw(win, lineY, 3, "%.*s", listWidth - 5, filesOnPage[i]->getFileName().c_str());
        
        wattroff(win, A_REVERSE | A_BOLD);
    }

    // --- Draw detail panel ---
    // Header with button
    std::string metaTitle = "Metadata";
    editButtonY = 2; // Same line as list header
    std::string editLabel = "[Edit]"; // Shorter label
    editButtonW = editLabel.length();
    int metaTitleX = listWidth + (detailWidth - metaTitle.length()) / 2; // Center title
    editButtonX = width - editButtonW - 2; // Align right

    mvwprintw(win, 2, metaTitleX, "%s", metaTitle.c_str());
    mvwprintw(win, editButtonY, editButtonX, "%s", editLabel.c_str());

    // Metadata content area border
    mvwhline(win, 3, listWidth + 1, ACS_HLINE, detailWidth - 2); // Top border
    mvwhline(win, height - 4, listWidth + 1, ACS_HLINE, detailWidth - 2); // Bottom border
    mvwvline(win, 4, listWidth, ACS_VLINE, height - 7); // Left border (relative to mainWin)
    mvwvline(win, 4, width - 2, ACS_VLINE, height - 7); // Right border
    // Corner pieces
    mvwaddch(win, 3, listWidth, ACS_ULCORNER);
    mvwaddch(win, 3, width - 2, ACS_URCORNER);
    mvwaddch(win, height - 4, listWidth, ACS_LLCORNER);
    mvwaddch(win, height - 4, width - 2, ACS_LRCORNER);


    // Metadata content - ONLY if explicitly selected
    if (fileExplicitlySelected) {
        MediaFile* selectedFile = getSelectedFile();
        if (selectedFile && selectedFile->getMetadata()) {
             mvwprintw(win, 4, listWidth + 2, "Title: %.*s", detailWidth-4, selectedFile->getMetadata()->title.c_str());
             // Draw other fields...
             mvwprintw(win, 5, listWidth + 2, "Artist: %.*s", detailWidth-4, selectedFile->getMetadata()->getField("artist").c_str());
             mvwprintw(win, 6, listWidth + 2, "Album: %.*s", detailWidth-4, selectedFile->getMetadata()->getField("album").c_str());

        } else {
             mvwprintw(win, 4, listWidth + 2, "(No metadata found)");
        }
    } else {
         mvwprintw(win, 4, listWidth + 2, "(Select a file to view details)"); // Initial message
    }


    // Add to Playlist Button (Centered at bottom of detail panel)
    addButtonY = height - 3; // Position near bottom
    std::string addLabel = "[Add to Playlist]";
    addButtonW = addLabel.length();
    addButtonX = listWidth + (detailWidth - addButtonW) / 2; // Center in detail panel

    // Highlight button if detail panel has focus
    if (focus == FocusArea::MAIN_DETAIL) wattron(win, A_REVERSE | A_BOLD);
    mvwprintw(win, addButtonY, addButtonX, "%s", addLabel.c_str());
    wattroff(win, A_REVERSE | A_BOLD);

    wnoutrefresh(win);
}
// --- END DRAW FUNCTION ---

// --- UPDATED HANDLEINPUT FUNCTION ---
MainAreaAction MainFileView::handleInput(InputEvent event, FocusArea focus) {
    if (!mediaManager) return MainAreaAction::NONE;
    int totalFiles = mediaManager->getTotalFileCount();
    bool selectionChanged = false; // Flag to check if selection actually moved

    if (focus == FocusArea::MAIN_LIST && totalFiles > 0) {

        if (event.key == KEY_DOWN) {
            fileSelected = std::min(fileSelected + 1, totalFiles - 1);
            selectionChanged = true;
        } else if (event.key == KEY_UP) {
            // Handle initial state where fileSelected might be -1
            if (fileSelected <= 0) fileSelected = 0;
            else fileSelected = std::max(0, fileSelected - 1);
            selectionChanged = true;
        } else if (event.key == KEY_PPAGE) { // Page Up
             if (filePage > 1) {
                 filePage--;
                 fileSelected = (filePage - 1) * itemsPerPage; // Select top item
                 selectionChanged = true;
             }
        } else if (event.key == KEY_NPAGE) { // Page Down
             if (filePage < totalPages) {
                 filePage++;
                 fileSelected = (filePage - 1) * itemsPerPage; // Select top item
                 fileSelected = std::min(fileSelected, totalFiles -1); // Clamp selection
                 selectionChanged = true;
             }
        }

        // Auto-scroll page if selection moved via UP/DOWN
        if (selectionChanged && (event.key == KEY_UP || event.key == KEY_DOWN)) {
             int newPage = (fileSelected < 0) ? 1 : (fileSelected / itemsPerPage) + 1;
             if (newPage != filePage) filePage = newPage;
        }

        // Set flag if user made a valid selection
        if (selectionChanged && fileSelected >= 0) {
            fileExplicitlySelected = true; // Mark as selected
        }


    } else if (focus == FocusArea::MAIN_DETAIL) {
        if (event.key == 10) { // Enter
            // Activate "Add to Playlist" if a file is explicitly selected
            if(fileExplicitlySelected && getSelectedFile() != nullptr) {
                return MainAreaAction::ADD_TRACK_TO_PLAYLIST;
            } else if (event.key == 'e' || event.key == 'E') {
                // Thêm phím 'e' (Edit) để kích hoạt
                if(fileExplicitlySelected && getSelectedFile() != nullptr) {
                return MainAreaAction::EDIT_METADATA;
                }
            }
            else {
                 flash(); // Indicate nothing selected
            }
        }

    }
    return MainAreaAction::NONE;
}
// --- END HANDLEINPUT FUNCTION ---

// --- UPDATED HANDLEMOUSE FUNCTION ---
MainAreaAction MainFileView::handleMouse(int localY, int localX) {
    if (!mediaManager) return MainAreaAction::NONE;
    int totalFiles = mediaManager->getTotalFileCount();
    int listStartY = 4;
    int clickedIndexOnPage = localY - listStartY;
    int width; getmaxyx(win, std::ignore, width); int listWidth = width / 2;

    // Check Header Button Clicks
    if (localY == prevBtnY) { // Clicked on header row
        if (localX >= prevBtnX && localX < prevBtnX + prevBtnW && filePage > 1) { // Prev
            filePage--;
            fileSelected = (filePage - 1) * itemsPerPage;
            if(fileSelected >= 0) fileExplicitlySelected = true;
            return MainAreaAction::NONE;
        }
        if (localX >= nextBtnX && localX < nextBtnX + nextBtnW && filePage < totalPages) { // Next
            filePage++;
            fileSelected = (filePage - 1) * itemsPerPage;
            fileSelected = std::min(fileSelected, totalFiles -1);
            if(fileSelected >= 0) fileExplicitlySelected = true;
            return MainAreaAction::NONE;
        }
        if (localX >= editButtonX && localX < editButtonX + editButtonW) { // Edit
             // Only allow editing if a file has been selected
             if (fileExplicitlySelected && getSelectedFile() != nullptr) {
                 return MainAreaAction::EDIT_METADATA;
             } else {
                 flash();
             }
        }
    }


    // Check List/Detail Panel Clicks
    if (localX < listWidth) { // Click on list panel
        if (clickedIndexOnPage >= 0 && clickedIndexOnPage < itemsPerPage) {
            int clickedIndexGlobal = (filePage - 1) * itemsPerPage + clickedIndexOnPage;
            if (clickedIndexGlobal < totalFiles) {
                fileSelected = clickedIndexGlobal;
                fileExplicitlySelected = true; // Mark as selected
            }
        }
    } else { // Click on detail panel
        // Check Add button click
        if (localY == addButtonY && localX >= addButtonX && localX < addButtonX + addButtonW) {
             if (fileExplicitlySelected && getSelectedFile() != nullptr) {
                return MainAreaAction::ADD_TRACK_TO_PLAYLIST;
             } else {
                 flash(); // Indicate no file selected
             }
        }

    }
    return MainAreaAction::NONE;
}
MediaFile* MainFileView::getSelectedFile() const {
    if (!mediaManager) return nullptr;
    int totalFiles = mediaManager->getTotalFileCount();
    if (fileSelected < 0 || fileSelected >= totalFiles) return nullptr;

    // Ensure itemsPerPage is valid before division
    if (itemsPerPage <= 0) {
        std::cerr << "Error: itemsPerPage is invalid in getSelectedFile: " << itemsPerPage << std::endl;
        return nullptr;
    }

    int targetPage = (fileSelected / itemsPerPage) + 1;
    int indexOnPage = fileSelected % itemsPerPage;

    // Recalculate totalPages for robust check
    int currentTotalPages = mediaManager->getTotalPages(itemsPerPage);
    if(currentTotalPages == 0) currentTotalPages = 1;

    if (targetPage < 1 || targetPage > currentTotalPages) {

         return nullptr; // Return null if page calc is wrong
    }

    std::vector<MediaFile*> pageData = mediaManager->getPage(targetPage, itemsPerPage);
    if (indexOnPage >= 0 && static_cast<size_t>(indexOnPage) < pageData.size()) {
        return pageData[indexOnPage];
    }


    if (static_cast<size_t>(indexOnPage) >= pageData.size()){
      std::cerr << "Error in getSelectedFile: indexOnPage=" << indexOnPage
                << " >= pageData.size=" << pageData.size() << std::endl;
    }
    return nullptr;
