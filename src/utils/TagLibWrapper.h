#pragma once
#include <string>
#include <memory>
#include "model/Metadata.h"

class TagLibWrapper {
public:
    TagLibWrapper();
    ~TagLibWrapper();

    std::unique_ptr<Metadata> readTags(const std::string& filePath);
    bool writeTags(const std::string& filePath, Metadata* metadata);
};