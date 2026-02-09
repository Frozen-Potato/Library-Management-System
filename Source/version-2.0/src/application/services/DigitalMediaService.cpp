#include "DigitalMediaService.h"
#include "src/utils/Exceptions.h"
#include "src/utils/DateTimeUtils.h"
#include <iostream>
#include <sstream>

DigitalMediaService::DigitalMediaService(std::shared_ptr<PostgresAdapter> db,
                                         std::shared_ptr<S3StorageClient> storage,
                                         std::shared_ptr<KafkaProducer> events,
                                         std::shared_ptr<RedisClient> cache)
    : db_(std::move(db)), storage_(std::move(storage)),
      events_(std::move(events)), cache_(std::move(cache)) {}

std::string DigitalMediaService::generateS3Key(long mediaId, const std::string& mimeType, int version) {
    std::string ext = "bin";
    if (mimeType == "application/pdf") ext = "pdf";
    else if (mimeType == "application/epub+zip") ext = "epub";
    else if (mimeType == "video/mp4") ext = "mp4";
    else if (mimeType == "audio/mpeg") ext = "mp3";
    else if (mimeType == "image/jpeg") ext = "jpg";
    else if (mimeType == "image/png") ext = "png";

    return "digital-media/" + std::to_string(mediaId) + "/v" + std::to_string(version) + "." + ext;
}

nlohmann::json DigitalMediaService::uploadMedia(const std::string& title,
                                                 const std::string& mimeType,
                                                 const std::string& fileData,
                                                 bool drmProtected) {
    if (title.empty())
        throw ValidationException("Title cannot be empty");
    if (fileData.empty())
        throw ValidationException("File data cannot be empty");

    // Create base media record (media_type_id 5 = DigitalMedia)
    long mediaId = db_->createMedia(5, title);
    std::string s3Key = generateS3Key(mediaId, mimeType, 1);

    // Upload to S3
    if (!storage_->uploadFile(s3Key, fileData, mimeType)) {
        throw DatabaseException("Failed to upload file to storage");
    }

    // Insert digital_media record via raw SQL through connection
    // For now, return metadata since we'd need to extend PostgresAdapter
    nlohmann::json metadata = {
        {"media_id", mediaId},
        {"title", title},
        {"mime_type", mimeType},
        {"s3_key", s3Key},
        {"file_size", fileData.size()},
        {"drm_protected", drmProtected},
        {"current_version", 1},
        {"created_at", nowToString()}
    };

    // Cache metadata
    cache_->setJson("digital_media:" + std::to_string(mediaId), metadata, 3600);

    // Publish event
    events_->produceJson("media.events", "media.uploaded", {
        {"event", "DIGITAL_MEDIA_UPLOADED"},
        {"media_id", mediaId},
        {"title", title},
        {"mime_type", mimeType},
        {"file_size", fileData.size()},
        {"timestamp", nowToString()}
    });

    return metadata;
}

std::string DigitalMediaService::getDownloadUrl(long mediaId, int expirySeconds) {
    if (mediaId <= 0)
        throw ValidationException("Invalid media ID");

    // Check cache for s3 key
    auto cached = cache_->getJson("digital_media:" + std::to_string(mediaId));
    std::string s3Key;
    if (cached.has_value() && cached->contains("s3_key")) {
        s3Key = (*cached)["s3_key"].get<std::string>();
    } else {
        throw NotFoundException("Digital media not found: " + std::to_string(mediaId));
    }

    return storage_->generatePresignedDownloadUrl(s3Key, expirySeconds);
}

std::string DigitalMediaService::getUploadUrl(long mediaId, const std::string& mimeType, int expirySeconds) {
    if (mediaId <= 0)
        throw ValidationException("Invalid media ID");

    auto cached = cache_->getJson("digital_media:" + std::to_string(mediaId));
    int nextVersion = 1;
    if (cached.has_value() && cached->contains("current_version")) {
        nextVersion = (*cached)["current_version"].get<int>() + 1;
    }

    std::string s3Key = generateS3Key(mediaId, mimeType, nextVersion);
    return storage_->generatePresignedUploadUrl(s3Key, expirySeconds);
}

nlohmann::json DigitalMediaService::createVersion(long mediaId, const std::string& fileData) {
    if (mediaId <= 0)
        throw ValidationException("Invalid media ID");
    if (fileData.empty())
        throw ValidationException("File data cannot be empty");

    auto cached = cache_->getJson("digital_media:" + std::to_string(mediaId));
    if (!cached.has_value())
        throw NotFoundException("Digital media not found: " + std::to_string(mediaId));

    int newVersion = (*cached)["current_version"].get<int>() + 1;
    std::string mimeType = (*cached)["mime_type"].get<std::string>();
    std::string s3Key = generateS3Key(mediaId, mimeType, newVersion);

    if (!storage_->uploadFile(s3Key, fileData, mimeType)) {
        throw DatabaseException("Failed to upload new version to storage");
    }

    // Update cached metadata
    (*cached)["current_version"] = newVersion;
    (*cached)["s3_key"] = s3Key;
    (*cached)["file_size"] = fileData.size();
    cache_->setJson("digital_media:" + std::to_string(mediaId), *cached, 3600);

    events_->produceJson("media.events", "media.versioned", {
        {"event", "DIGITAL_MEDIA_VERSION_CREATED"},
        {"media_id", mediaId},
        {"version", newVersion},
        {"timestamp", nowToString()}
    });

    return {
        {"media_id", mediaId},
        {"version", newVersion},
        {"s3_key", s3Key},
        {"file_size", fileData.size()}
    };
}

std::vector<nlohmann::json> DigitalMediaService::listVersions(long mediaId) {
    if (mediaId <= 0)
        throw ValidationException("Invalid media ID");
    return storage_->listVersions("digital-media/" + std::to_string(mediaId) + "/");
}

std::optional<nlohmann::json> DigitalMediaService::getMetadata(long mediaId) {
    return cache_->getJson("digital_media:" + std::to_string(mediaId));
}

bool DigitalMediaService::deleteMedia(long mediaId) {
    auto cached = cache_->getJson("digital_media:" + std::to_string(mediaId));
    if (cached.has_value() && cached->contains("s3_key")) {
        storage_->deleteFile((*cached)["s3_key"].get<std::string>());
    }
    cache_->del("digital_media:" + std::to_string(mediaId));

    events_->produceJson("media.events", "media.deleted", {
        {"event", "DIGITAL_MEDIA_DELETED"},
        {"media_id", mediaId},
        {"timestamp", nowToString()}
    });

    return true;
}
