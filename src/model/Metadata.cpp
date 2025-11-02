#include "model/Metadata.h"

Metadata::Metadata() : durationInSeconds(0) {
    // initialize
    fields["artist"] = "";
    fields["album"] = "";
    fields["genre"] = "";
    fields["year"] = "0";
}


std::string Metadata::getField(const std::string& key) const {
    if (key == "title") {
        return this->title;
    }
    
    auto it = this->fields.find(key);
    if (it != this->fields.end()) {
        return it->second;
    }
    
    return ""; // Return empty string if not found
}

void Metadata::setField(const std::string& key, const std::string& value) {
    if (key == "title") {
        this->title = value;
    }
    
    this->fields[key] = value;
}