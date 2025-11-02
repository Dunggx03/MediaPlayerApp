#include "model/MediaManager.h"
#include "utils/TagLibWrapper.h" // We need the real wrapper
#include <iostream>
#include <cassert>
#include <memory>
#include <cmath>

/**
 * ================== !! QUAN TRỌNG !! ==================
 * This is an INTEGRATION TEST, not a unit test.
 * * To run this test, you MUST:
 * 1. Create a directory named 'test_media' in the project root.
 * 2. Place 1 or 2 real .mp3 files (with valid tags) inside 'test_media/'.
 * * This test uses the real FileUtils and TagLibWrapper.
 * =======================================================
 */

int main() {
    std::cout << "🧪 Running tests for MediaManager..." << std::endl;
    
    // 1. Setup real dependencies
    TagLibWrapper tagUtil;
    MediaManager mm(&tagUtil);

    // --- Test: Initial state ---
    assert(mm.getTotalFileCount() == 0);
    assert(mm.getTotalPages() == 1);
    assert(mm.getPage(1).size() == 0);

    // --- Test: loadFromDirectory ---
    std::string testPath = "./test_media"; 
    mm.loadFromDirectory(testPath);

    // NOTE: If this assert fails, make sure your 'test_media' directory
    // contains valid .mp3 files that TagLib can read.
    assert(mm.getTotalFileCount() > 0); 
    
    int fileCount = mm.getTotalFileCount();
    std::cout << "  > Found " << fileCount << " files in " << testPath << std::endl;

    // --- Test: Pagination logic (with 2 items per page) ---
    int pageSize = 2;
    int expectedPages = static_cast<int>(std::ceil(static_cast<double>(fileCount) / pageSize));
    assert(mm.getTotalPages(pageSize) == expectedPages);

    // Get page 1
    std::vector<MediaFile*> page1 = mm.getPage(1, pageSize);
    //assert(page1.size() == std::min(fileCount, pageSize));
    assert(page1.size() == static_cast<size_t>(std::min(fileCount, pageSize)));

    // Check file data
    assert(page1[0]->getMetadata() != nullptr);
    assert(!page1[0]->getFileName().empty());
    assert(!page1[0]->getMetadata()->title.empty()); // TagLibWrapper should set this

    std::cout << "  > Page 1, Item 1: " << page1[0]->getFileName() << std::endl;

    // --- Test: clearLibrary ---
    mm.clearLibrary();
    assert(mm.getTotalFileCount() == 0);
    assert(mm.getTotalPages(pageSize) == 1);
    assert(mm.getPage(1, pageSize).empty() == true);

    std::cout << "✅ MediaManager tests passed!" << std::endl;
    return 0;
}