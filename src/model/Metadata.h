#pragma once
#include <string>
#include <map>

class Metadata {
public:
    Metadata();
    virtual ~Metadata() = default;

    std::string title;
    int durationInSeconds = 0;
    long fileSizeInBytes = 0;

    
    virtual std::string getField(const std::string& key) const;
    void setField(const std::string& key, const std::string& value);
protected:
    std::map<std::string, std::string> fields;
};