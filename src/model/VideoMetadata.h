#pragma once
#include "Metadata.h"

class VideoMetadata : public Metadata {
public:
    std::string resolution; 
    int bitrateKbps;   
    std::string codec;
};