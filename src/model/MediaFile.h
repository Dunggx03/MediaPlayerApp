#pragma once
#include <string>
#include <memory> 
#include "Metadata.h"
#include "AudioMetadata.h" 
#include "VideoMetadata.h" 

enum class MediaType { UNKNOWN, AUDIO, VIDEO };

class MediaFile {
public:
    MediaFile(const std::string& path, std::unique_ptr<Metadata> metadata);

    const std::string& getFilePath() const;
    const std::string& getFileName() const; 
    MediaType getType() const;
    Metadata* getMetadata() const; 

private:
    std::string filePath;
    std::string fileName;
    MediaType mediaType;
    std::unique_ptr<Metadata> metadata;
};