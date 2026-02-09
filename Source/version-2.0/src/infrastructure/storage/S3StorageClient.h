#pragma once
#include <string>
#include <optional>
#include <vector>
#include <mutex>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

struct S3Object {
    std::string key;
    std::string contentType;
    size_t size;
    std::string lastModified;
    std::string etag;
};

class S3StorageClient {
public:
    S3StorageClient(const std::string& endpoint,
                    const std::string& accessKey,
                    const std::string& secretKey,
                    const std::string& bucket,
                    const std::string& region = "us-east-1");
    ~S3StorageClient();

    // Upload/Download
    bool uploadFile(const std::string& key,
                    const std::string& data,
                    const std::string& contentType = "application/octet-stream");
    std::optional<std::string> downloadFile(const std::string& key);
    bool deleteFile(const std::string& key);

    // Presigned URLs
    std::string generatePresignedUploadUrl(const std::string& key, int expirySeconds = 3600);
    std::string generatePresignedDownloadUrl(const std::string& key, int expirySeconds = 3600);

    // Listing and metadata
    std::vector<S3Object> listObjects(const std::string& prefix = "", int maxKeys = 1000);
    std::optional<S3Object> headObject(const std::string& key);

    // Versioning
    bool enableVersioning();
    std::vector<nlohmann::json> listVersions(const std::string& key);

    // Health
    bool bucketExists();

private:
    std::string signV4(const std::string& method, const std::string& path,
                       const std::string& queryString, const std::string& payloadHash,
                       const std::string& contentType, const std::string& timestamp);
    std::string hmacSha256(const std::string& key, const std::string& data);
    std::string sha256(const std::string& data);
    std::string getTimestamp();
    std::string getDateStamp();

    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);

    std::string endpoint_;
    std::string accessKey_;
    std::string secretKey_;
    std::string bucket_;
    std::string region_;
    CURL* curl_;
    std::mutex mtx_;
};
