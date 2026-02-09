#pragma once
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include "src/data/PostgresAdapter.h"
#include "src/infrastructure/storage/S3StorageClient.h"
#include "src/infrastructure/messaging/KafkaProducer.h"
#include "src/infrastructure/cache/RedisClient.h"
#include "src/domain/media/DigitalMedia.h"

struct DigitalMediaRecord {
    long id;
    long mediaId;
    std::string mimeType;
    std::string s3Key;
    size_t fileSize;
    bool drmProtected;
    int currentVersion;
};

class DigitalMediaService {
public:
    DigitalMediaService(std::shared_ptr<PostgresAdapter> db,
                        std::shared_ptr<S3StorageClient> storage,
                        std::shared_ptr<KafkaProducer> events,
                        std::shared_ptr<RedisClient> cache);

    // Upload digital media (creates media + digital_media record + uploads to S3)
    nlohmann::json uploadMedia(const std::string& title,
                               const std::string& mimeType,
                               const std::string& fileData,
                               bool drmProtected = false);

    // Generate presigned download URL
    std::string getDownloadUrl(long mediaId, int expirySeconds = 3600);

    // Generate presigned upload URL for direct client upload
    std::string getUploadUrl(long mediaId, const std::string& mimeType, int expirySeconds = 3600);

    // Create a new version of existing digital media
    nlohmann::json createVersion(long mediaId, const std::string& fileData);

    // List versions for a media item
    std::vector<nlohmann::json> listVersions(long mediaId);

    // Get metadata
    std::optional<nlohmann::json> getMetadata(long mediaId);

    // Delete digital media (file + record)
    bool deleteMedia(long mediaId);

private:
    std::string generateS3Key(long mediaId, const std::string& mimeType, int version = 1);

    std::shared_ptr<PostgresAdapter> db_;
    std::shared_ptr<S3StorageClient> storage_;
    std::shared_ptr<KafkaProducer> events_;
    std::shared_ptr<RedisClient> cache_;
};
