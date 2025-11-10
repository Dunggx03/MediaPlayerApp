#include "TagLibWrapper.h"
#include "model/AudioMetadata.h" 
#include "model/VideoMetadata.h"
#include <iostream>
#include <filesystem> 
#include <string>     
#include <algorithm>  

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include <taglib/audioproperties.h>

namespace fs = std::filesystem;

namespace {
    std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        return s;
    }
}

TagLibWrapper::TagLibWrapper() {}

TagLibWrapper::~TagLibWrapper() {}

std::unique_ptr<Metadata> TagLibWrapper::readTags(const std::string& filePath) {
    TagLib::FileRef f(filePath.c_str());

    if (f.isNull()) {
        std::cerr << "TagLibWrapper: Could not read file: " << filePath << std::endl;
        return nullptr;
    }

    // Create a new AudioMetadata object (or base Metadata if no audio)
    std::unique_ptr<Metadata> meta;
    if (f.audioProperties()) {
        meta = std::make_unique<AudioMetadata>();
    } else {
        meta = std::make_unique<Metadata>(); // Basic metadata
    }

    // --- Fill common Metadata fields ---
    meta->fileSizeInBytes = f.file()->length();
    
    if(f.tag()) { // Check if tag exists
        TagLib::Tag* tag = f.tag();
        if (tag->title().isEmpty()) {
            // Fallback: use the file name (without extension) as the title
            meta->title = fs::path(filePath).stem().string();
        } else {
            meta->title = tag->title().toCString(true);
        }
    } else {
        meta->title = fs::path(filePath).stem().string(); // Fallback if no tag block
    }

    if(f.audioProperties()) { // Check if audio properties exist
        meta->durationInSeconds = f.audioProperties()->lengthInSeconds();
    }

    // --- Fill map 'fields' using setField ---
    if (f.tag()) {
        TagLib::PropertyMap allTags = f.tag()->properties();
        
        // Use setField for standard fields to ensure map is populated
        meta->setField("artist", f.tag()->artist().toCString(true));
        meta->setField("album", f.tag()->album().toCString(true));
        meta->setField("genre", f.tag()->genre().toCString(true));
        meta->setField("year", std::to_string(f.tag()->year())); // Convert year to string

        // Add any other properties found
        for(auto const& [key, valList] : allTags) {
            if (!valList.isEmpty()) {
                // Convert key to lowercase to be consistent (e.g., "ARTIST" -> "artist")
                std::string lKey = toLower(key.toCString(true));
                
                // Only add if we haven't already set it (optional)
                if (meta->getField(lKey).empty()) { 
                    std::string lVal = valList.front().toCString(true);
                    meta->setField(lKey, lVal);
                }
            }
        }
    }

    return meta; // Return the new object
}

bool TagLibWrapper::writeTags(const std::string& filePath, Metadata* metadata) {
    if (metadata == nullptr) {
        std::cerr << "TagLibWrapper Error: Cannot write null metadata." << std::endl;
        return false;
    }

    TagLib::FileRef f(filePath.c_str());
    if (f.isNull() || !f.tag()) {
        std::cerr << "TagLibWrapper Error: Could not open file for writing: " << filePath << std::endl;
        return false;
    }

    TagLib::Tag *tag = f.tag();

    tag->setTitle(TagLib::String(metadata->title, TagLib::String::UTF8));
    tag->setArtist(TagLib::String(metadata->getField("artist"), TagLib::String::UTF8));
    tag->setAlbum(TagLib::String(metadata->getField("album"), TagLib::String::UTF8));
    tag->setGenre(TagLib::String(metadata->getField("genre"), TagLib::String::UTF8));
    
    // Convert year from string back to uint
    try {
        std::string yearStr = metadata->getField("year");
        if (yearStr.empty()) {
            tag->setYear(0);
        } else {
            tag->setYear(std::stoi(yearStr));
        }
    } catch (...) {
        tag->setYear(0); // Set to 0 if conversion fails
    }

    if (f.save()) {
        std::cout << "TagLibWrapper: Metadata saved successfully for: " << filePath << std::endl;
        return true;
    } else {
        std::cerr << "TagLibWrapper Error: Failed to save metadata for: " << filePath << std::endl;
        return false;
    }
}