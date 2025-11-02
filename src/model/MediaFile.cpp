#include "model/MediaFile.h"
#include <filesystem>

MediaFile::MediaFile(const std::string& path, std::unique_ptr<Metadata> metadata)
    : filePath(path), metadata(std::move(metadata))
{
    try {
        this->fileName = std::filesystem::path(path).filename().string();
    } catch (std::exception& e) {
        this->fileName = path; // Fallback
    }

    if (dynamic_cast<AudioMetadata*>(this->metadata.get())) {
        this->mediaType = MediaType::AUDIO;
    } else if (dynamic_cast<VideoMetadata*>(this->metadata.get())) {
        this->mediaType = MediaType::VIDEO;
    } else {
        this->mediaType = MediaType::UNKNOWN;
    }
}

const std::string& MediaFile::getFilePath() const {
    return this->filePath;
}

const std::string& MediaFile::getFileName() const {
    return this->fileName;
}

MediaType MediaFile::getType() const {
    return this->mediaType;
}

Metadata* MediaFile::getMetadata() const {
    return metadata.get(); // Return the raw pointer
}