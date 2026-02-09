#pragma once
#include "Media.h"
#include <chrono>
#include <vector>

struct FileVersion {
    int versionNumber;
    std::string s3Key;
    size_t fileSize;
    std::string checksum;
    std::string uploadedAt;
    bool isCurrent;
};

class DigitalMedia : public Media {
public:
    DigitalMedia(long id, const std::string& title,
                 const std::string& mimeType,
                 const std::string& s3Key,
                 size_t fileSize,
                 bool drmProtected = false,
                 int currentVersion = 1)
        : Media(id, title), mimeType_(mimeType), s3Key_(s3Key),
          fileSize_(fileSize), drmProtected_(drmProtected),
          currentVersion_(currentVersion) {}

    std::string getType() const override { return "DigitalMedia"; }

    void print(std::ostream& os) const override {
        os << "[DigitalMedia] " << title_ << " (" << mimeType_ << ", "
           << fileSize_ << " bytes, v" << currentVersion_ << ")";
    }

    const std::string& getMimeType() const { return mimeType_; }
    const std::string& getS3Key() const { return s3Key_; }
    size_t getFileSize() const { return fileSize_; }
    bool isDrmProtected() const { return drmProtected_; }
    int getCurrentVersion() const { return currentVersion_; }

    void setS3Key(const std::string& key) { s3Key_ = key; }
    void setCurrentVersion(int v) { currentVersion_ = v; }
    void setFileSize(size_t s) { fileSize_ = s; }

private:
    std::string mimeType_;
    std::string s3Key_;
    size_t fileSize_;
    bool drmProtected_;
    int currentVersion_;
};
