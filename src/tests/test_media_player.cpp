#include "model/MediaPlayer.h"
#include "utils/SDLWrapper.h"
#include "utils/TagLibWrapper.h" // Cần để tạo MediaFile
#include "model/MediaFile.h"
#include "model/Metadata.h"
#include <iostream>
#include <cassert>
#include <unistd.h> // For sleep()

/**
 * ================== !! QUAN TRỌNG !! ==================
 * This is an INTEGRATION TEST.
 * 1. It assumes 'test_media/file1.mp3' exists.
 * 2. It WILL play sound for 1 second.
 * =======================================================
 */

// Helper to create a real MediaFile
std::unique_ptr<MediaFile> loadMockFile(TagLibWrapper* tagUtil, const std::string& path) {
    std::unique_ptr<Metadata> metadata = tagUtil->readTags(path);
    if (metadata) {
        return std::make_unique<MediaFile>(path, std::move(metadata));
    }
    return nullptr;
}

int main() {
    std::cout << "🧪 Running tests for MediaPlayer..." << std::endl;
    
    // 1. Setup real dependencies
    TagLibWrapper tagUtil;
    SDLWrapper sdl;
    assert(sdl.init() == true); // Phải khởi tạo SDL
    
    MediaPlayer player(&sdl);

    // 2. Load a real test file
    auto file = loadMockFile(&tagUtil, "test_media/Background 1.mp3");
    assert(file != nullptr); // Dừng nếu không tìm thấy file
    assert(file->getMetadata()->durationInSeconds > 0);

    // --- Test: Initial state ---
    assert(player.getState() == PlayerState::STOPPED);
    assert(player.getCurrentTrack() == nullptr);
    assert(player.getCurrentTime() == 0);
    assert(player.getTotalTime() == 0);

    // --- Test: Play ---
    player.play(file.get());
    assert(player.getState() == PlayerState::PLAYING);
    assert(player.getCurrentTrack() == file.get());
    assert(player.getTotalTime() == file->getMetadata()->durationInSeconds);

    std::cout << "  > Playing audio for 1 second..." << std::endl;
    sleep(1); // Chờ 1 giây
    assert(player.getCurrentTime() >= 1); // Phải đang phát

    // --- Test: Pause ---
    player.pause();
    assert(player.getState() == PlayerState::PAUSED);
    int pausedTime = player.getCurrentTime();
    std::cout << "  > Paused at " << pausedTime << "s. Waiting 1 sec..." << std::endl;
    sleep(1);
    assert(player.getCurrentTime() == pausedTime); // Thời gian không được trôi

    // --- Test: Resume (un-pause) ---
    player.pause();
    assert(player.getState() == PlayerState::PLAYING);
    std::cout << "  > Resumed. Waiting 1 sec..." << std::endl;
    sleep(1);
    assert(player.getCurrentTime() > pausedTime); // Thời gian phải trôi

    // --- Test: Stop ---
    player.stop();
    assert(player.getState() == PlayerState::STOPPED);
    assert(player.getCurrentTrack() == nullptr);
    assert(player.getCurrentTime() == 0);
    assert(player.getTotalTime() == 0); // TotalTime cũng reset

    std::cout << "✅ MediaPlayer tests passed!" << std::endl;
    sdl.close();
    return 0;
}