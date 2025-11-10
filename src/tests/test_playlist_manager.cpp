#include "model/PlaylistManager.h"
#include <iostream>
#include <cassert> // For basic testing

int main() {
    std::cout << "🧪 Running tests for PlaylistManager..." << std::endl;
    
    PlaylistManager pm;

    // --- Test: Initial state ---
    assert(pm.getAllPlaylists().size() == 0);
    assert(pm.getPlaylistByName("Favorites") == nullptr);

    // --- Test: createPlaylist ---
    Playlist* p1 = pm.createPlaylist("Favorites");
    assert(p1 != nullptr);
    assert(p1->getName() == "Favorites");
    
    // --- Test: getAllPlaylists ---
    assert(pm.getAllPlaylists().size() == 1);
    assert(pm.getAllPlaylists()[0]->getName() == "Favorites");

    // --- Test: Duplicate creation ---
    Playlist* p2 = pm.createPlaylist("Favorites");
    assert(p2 == nullptr); // Should fail, name already exists
    assert(pm.getAllPlaylists().size() == 1); // Size should not change

    // --- Test: Create another ---
    Playlist* p3 = pm.createPlaylist("Chill");
    assert(p3 != nullptr);
    assert(pm.getAllPlaylists().size() == 2);

    // --- Test: getPlaylistByName ---
    assert(pm.getPlaylistByName("Favorites") == p1);
    assert(pm.getPlaylistByName("Chill") == p3);
    assert(pm.getPlaylistByName("NonExistent") == nullptr);

    // --- Test: deletePlaylist ---
    bool success = pm.deletePlaylist("Favorites");
    assert(success == true);
    assert(pm.getAllPlaylists().size() == 1);
    assert(pm.getPlaylistByName("Favorites") == nullptr); 
    assert(pm.getPlaylistByName("Chill") == p3);

    // --- Test: Delete non-existent ---
    bool fail = pm.deletePlaylist("Favorites");
    assert(fail == false);
    assert(pm.getAllPlaylists().size() == 1);

    std::cout << "✅ PlaylistManager tests passed!" << std::endl;
    return 0;
}