#include "../../include/model/MediaFile.h"
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/textidentificationframe.h>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;
    MediaFile::MediaFile(const std::string& path) : metadata(path.c_str()), Path(path) {
        name = fs::path(path).filename().string();
    }

    std::string MediaFile::getName() const {
        return name;
    }

    std::string MediaFile::getPath() const {
        return Path;
    }

    std::string MediaFile::getTitle() const {
        if (!metadata.isNull() && metadata.tag()) {
            return metadata.tag()->title().to8Bit(true);
        }
        return "";
    }

    std::string MediaFile::getArtist() const {
        if (!metadata.isNull() && metadata.tag()) {
            return metadata.tag()->artist().to8Bit(true);
        }
        return "";
    }

    std::string MediaFile::getAlbum() const {
        if (!metadata.isNull() && metadata.tag()) {
            return metadata.tag()->album().to8Bit(true);
        }
        return "";
    }

    std::string MediaFile::getGenre() const {
        if (!metadata.isNull() && metadata.tag()) {
            return metadata.tag()->genre().to8Bit(true);
        }
        return "";
    }

    int MediaFile::getDuration() const {
        if (!metadata.isNull() && metadata.audioProperties()) {
            return metadata.audioProperties()->length();
        }
        return 0;
    }

    unsigned int MediaFile::getYear() const {
        if (!metadata.isNull() && metadata.tag()) {
            return metadata.tag()->year();
        }
        return 0;
    }

    unsigned int MediaFile::getTrack() const {
        if (!metadata.isNull() && metadata.tag()) {
            return metadata.tag()->track();
        }
        return 0;
    }

    bool MediaFile::setMetadata(const std::string& key, const std::string& value) {
        // handle common standard fields directly; otherwise fall through to create a custom TXXX frame
        std::string lkey = key;
        for (char &ch : lkey) {
            if (ch >= 'A' && ch <= 'Z') ch = ch - 'A' + 'a';
        }

        if (lkey == "title" || lkey == "artist" || lkey == "album" ||
            lkey == "genre" || lkey == "comment" || lkey == "year" || lkey == "track") {

            TagLib::MPEG::File mp3File(Path.c_str());
            if (!mp3File.isValid()) {
                std::cerr << "Invalid File!\n";
                return false;
            }

            TagLib::ID3v2::Tag *tag = mp3File.ID3v2Tag(true);

            if (lkey == "title") {
                tag->setTitle(TagLib::String(value, TagLib::String::UTF8));
            } else if (lkey == "artist") {
                tag->setArtist(TagLib::String(value, TagLib::String::UTF8));
            } else if (lkey == "album") {
                tag->setAlbum(TagLib::String(value, TagLib::String::UTF8));
            } else if (lkey == "genre") {
                tag->setGenre(TagLib::String(value, TagLib::String::UTF8));
            } else if (lkey == "comment") {
                tag->setComment(TagLib::String(value, TagLib::String::UTF8));
            } else if (lkey == "year") {
                try {
                    unsigned int y = static_cast<unsigned int>(std::stoul(value));
                    tag->setYear(y);
                } catch (...) {
                    std::cerr << "Invalid year value\n";
                    return false;
                }
            } else if (lkey == "track") {
                try {
                    unsigned int t = static_cast<unsigned int>(std::stoul(value));
                    tag->setTrack(t);
                } catch (...) {
                    std::cerr << "Invalid track value\n";
                    return false;
                }
            }

            mp3File.save();
            metadata = TagLib::FileRef(Path.c_str());
            return true;
        }
        else {
            TagLib::MPEG::File file(Path.c_str());

            if (!file.isValid()) {
                std::cerr << "Invalid File!\n";
                return false;
            }

            TagLib::ID3v2::Tag *tag = file.ID3v2Tag(true);


            auto *frame = new TagLib::ID3v2::TextIdentificationFrame("TXXX", TagLib::String::UTF8);

            // Tạo danh sách 2 phần: [description, value]
            TagLib::StringList fields;
            fields.append(key);     // phần mô tả
            fields.append(value);    // phần giá trị
            frame->setText(fields);

            // Thêm frame vào tag
            tag->addFrame(frame);
            file.save();
            metadata = TagLib::FileRef(Path.c_str());
            return true;
        }
    }

    std::string MediaFile::getCustomMetadataField(const std::string& key) const {
        TagLib::MPEG::File file(Path.c_str());

        if (!file.isValid()) {
            std::cerr << "Invalid File!\n";
            return "";
        }

        TagLib::ID3v2::Tag *tag = file.ID3v2Tag(true);
        for (auto *f : tag->frameList("TXXX")) {
            auto *tframe = dynamic_cast<TagLib::ID3v2::TextIdentificationFrame*>(f);
            if (tframe) {
                TagLib::StringList list = tframe->fieldList();
                if (list.size() >= 2 && list[0] == key) {
                    return list[1].to8Bit(true);
                }
            }
        }
        return "";
    }

    bool MediaFile::isValid() const {
        TagLib::FileRef f(Path.c_str());
        return !f.isNull() && f.tag();
    }
    std::string MediaFile::getType() const {
        return type;
    }