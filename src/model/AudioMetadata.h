#pragma once
#include "Metadata.h"

class AudioMetadata : public Metadata {
public:
    std::string artist;
    std::string album;
    std::string genre;
    int year;
    std::string publisher;
};