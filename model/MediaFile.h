#ifndef MEDIAFILE_H
#define MEDIAFILE_H

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <string>

class MediaFile {
private:
    TagLib::FileRef metadata;
    std::string Path;
    std::string name;
    std::string type;
public:
    MediaFile(const std::string& path);

    std::string getName() const;

    std::string getType() const;

    std::string getPath() const;

    std::string getTitle() const;

    std::string getArtist() const;

    std::string getAlbum() const;

    std::string getGenre() const;

    int getDuration() const;

    unsigned int getYear() const;

    unsigned int getTrack() const;

    bool setMetadata(const std::string& key, const std::string& value);

    bool isValid() const;

    std::string getCustomMetadataField(const std::string& key) const;
};

#endif // MEDIAFILE_H