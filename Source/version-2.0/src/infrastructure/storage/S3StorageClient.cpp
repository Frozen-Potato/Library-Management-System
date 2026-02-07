#include "S3StorageClient.h"
#include <iostream>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <algorithm>

S3StorageClient::S3StorageClient(const std::string& endpoint,
                                 const std::string& accessKey,
                                 const std::string& secretKey,
                                 const std::string& bucket,
                                 const std::string& region)
    : endpoint_(endpoint), accessKey_(accessKey), secretKey_(secretKey),
      bucket_(bucket), region_(region), curl_(nullptr) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_ = curl_easy_init();
}

S3StorageClient::~S3StorageClient() {
    if (curl_) curl_easy_cleanup(curl_);
    curl_global_cleanup();
}

size_t S3StorageClient::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realSize = size * nmemb;
    reinterpret_cast<std::string*>(userp)->append(static_cast<char*>(contents), realSize);
    return realSize;
}

std::string S3StorageClient::sha256(const std::string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), hash);
    std::ostringstream os;
    for (unsigned char c : hash)
        os << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(c);
    return os.str();
}

std::string S3StorageClient::hmacSha256(const std::string& key, const std::string& data) {
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    HMAC(EVP_sha256(),
         key.c_str(), static_cast<int>(key.size()),
         reinterpret_cast<const unsigned char*>(data.c_str()), data.size(),
         result, &len);
    return std::string(reinterpret_cast<char*>(result), len);
}

std::string S3StorageClient::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    gmtime_r(&t, &tm);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y%m%dT%H%M%SZ", &tm);
    return std::string(buf);
}

std::string S3StorageClient::getDateStamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    gmtime_r(&t, &tm);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y%m%d", &tm);
    return std::string(buf);
}

std::string S3StorageClient::signV4(const std::string& method, const std::string& path,
                                     const std::string& queryString, const std::string& payloadHash,
                                     const std::string& contentType, const std::string& timestamp) {
    std::string dateStamp = timestamp.substr(0, 8);
    std::string host = endpoint_;
    // Strip protocol
    if (host.find("://") != std::string::npos)
        host = host.substr(host.find("://") + 3);
    // Strip trailing slash
    if (!host.empty() && host.back() == '/') host.pop_back();

    std::string canonicalHeaders = "content-type:" + contentType + "\n"
                                  + "host:" + host + "\n"
                                  + "x-amz-content-sha256:" + payloadHash + "\n"
                                  + "x-amz-date:" + timestamp + "\n";
    std::string signedHeaders = "content-type;host;x-amz-content-sha256;x-amz-date";

    std::string canonicalRequest = method + "\n"
                                  + path + "\n"
                                  + queryString + "\n"
                                  + canonicalHeaders + "\n"
                                  + signedHeaders + "\n"
                                  + payloadHash;

    std::string scope = dateStamp + "/" + region_ + "/s3/aws4_request";
    std::string stringToSign = "AWS4-HMAC-SHA256\n" + timestamp + "\n" + scope + "\n" + sha256(canonicalRequest);

    std::string kDate = hmacSha256("AWS4" + secretKey_, dateStamp);
    std::string kRegion = hmacSha256(kDate, region_);
    std::string kService = hmacSha256(kRegion, "s3");
    std::string kSigning = hmacSha256(kService, "aws4_request");
    std::string signature = hmacSha256(kSigning, stringToSign);

    // Hex-encode signature
    std::ostringstream sigHex;
    for (unsigned char c : signature)
        sigHex << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(c);

    return "AWS4-HMAC-SHA256 Credential=" + accessKey_ + "/" + scope
         + ", SignedHeaders=" + signedHeaders
         + ", Signature=" + sigHex.str();
}

bool S3StorageClient::uploadFile(const std::string& key,
                                  const std::string& data,
                                  const std::string& contentType) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!curl_) return false;
    curl_easy_reset(curl_);

    std::string path = "/" + bucket_ + "/" + key;
    std::string url = endpoint_ + path;
    std::string timestamp = getTimestamp();
    std::string payloadHash = sha256(data);
    std::string auth = signV4("PUT", path, "", payloadHash, contentType, timestamp);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: " + auth).c_str());
    headers = curl_slist_append(headers, ("x-amz-date: " + timestamp).c_str());
    headers = curl_slist_append(headers, ("x-amz-content-sha256: " + payloadHash).c_str());
    headers = curl_slist_append(headers, ("Content-Type: " + contentType).c_str());

    std::string response;
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, data.size());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl_);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        std::cerr << "[S3] Upload failed: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    long httpCode = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &httpCode);
    return httpCode >= 200 && httpCode < 300;
}

std::optional<std::string> S3StorageClient::downloadFile(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!curl_) return std::nullopt;
    curl_easy_reset(curl_);

    std::string path = "/" + bucket_ + "/" + key;
    std::string url = endpoint_ + path;
    std::string timestamp = getTimestamp();
    std::string payloadHash = sha256("");
    std::string auth = signV4("GET", path, "", payloadHash, "application/octet-stream", timestamp);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: " + auth).c_str());
    headers = curl_slist_append(headers, ("x-amz-date: " + timestamp).c_str());
    headers = curl_slist_append(headers, ("x-amz-content-sha256: " + payloadHash).c_str());

    std::string response;
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 60L);

    CURLcode res = curl_easy_perform(curl_);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        std::cerr << "[S3] Download failed: " << curl_easy_strerror(res) << std::endl;
        return std::nullopt;
    }

    long httpCode = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &httpCode);
    if (httpCode != 200) return std::nullopt;
    return response;
}

bool S3StorageClient::deleteFile(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!curl_) return false;
    curl_easy_reset(curl_);

    std::string path = "/" + bucket_ + "/" + key;
    std::string url = endpoint_ + path;
    std::string timestamp = getTimestamp();
    std::string payloadHash = sha256("");
    std::string auth = signV4("DELETE", path, "", payloadHash, "application/octet-stream", timestamp);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: " + auth).c_str());
    headers = curl_slist_append(headers, ("x-amz-date: " + timestamp).c_str());
    headers = curl_slist_append(headers, ("x-amz-content-sha256: " + payloadHash).c_str());

    std::string response;
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl_);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) return false;

    long httpCode = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &httpCode);
    return httpCode >= 200 && httpCode < 300;
}

std::string S3StorageClient::generatePresignedDownloadUrl(const std::string& key, int expirySeconds) {
    std::string timestamp = getTimestamp();
    std::string dateStamp = timestamp.substr(0, 8);
    std::string path = "/" + bucket_ + "/" + key;
    std::string scope = dateStamp + "/" + region_ + "/s3/aws4_request";

    std::string queryString =
        "X-Amz-Algorithm=AWS4-HMAC-SHA256"
        "&X-Amz-Credential=" + accessKey_ + "/" + scope +
        "&X-Amz-Date=" + timestamp +
        "&X-Amz-Expires=" + std::to_string(expirySeconds) +
        "&X-Amz-SignedHeaders=host";

    std::string host = endpoint_;
    if (host.find("://") != std::string::npos)
        host = host.substr(host.find("://") + 3);
    if (!host.empty() && host.back() == '/') host.pop_back();

    std::string canonicalRequest = "GET\n" + path + "\n" + queryString + "\n"
                                  + "host:" + host + "\n\nhost\nUNSIGNED-PAYLOAD";
    std::string stringToSign = "AWS4-HMAC-SHA256\n" + timestamp + "\n" + scope + "\n" + sha256(canonicalRequest);

    std::string kDate = hmacSha256("AWS4" + secretKey_, dateStamp);
    std::string kRegion = hmacSha256(kDate, region_);
    std::string kService = hmacSha256(kRegion, "s3");
    std::string kSigning = hmacSha256(kService, "aws4_request");
    std::string signature = hmacSha256(kSigning, stringToSign);

    std::ostringstream sigHex;
    for (unsigned char c : signature)
        sigHex << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(c);

    return endpoint_ + path + "?" + queryString + "&X-Amz-Signature=" + sigHex.str();
}

std::string S3StorageClient::generatePresignedUploadUrl(const std::string& key, int expirySeconds) {
    std::string timestamp = getTimestamp();
    std::string dateStamp = timestamp.substr(0, 8);
    std::string path = "/" + bucket_ + "/" + key;
    std::string scope = dateStamp + "/" + region_ + "/s3/aws4_request";

    std::string queryString =
        "X-Amz-Algorithm=AWS4-HMAC-SHA256"
        "&X-Amz-Credential=" + accessKey_ + "/" + scope +
        "&X-Amz-Date=" + timestamp +
        "&X-Amz-Expires=" + std::to_string(expirySeconds) +
        "&X-Amz-SignedHeaders=host";

    std::string host = endpoint_;
    if (host.find("://") != std::string::npos)
        host = host.substr(host.find("://") + 3);
    if (!host.empty() && host.back() == '/') host.pop_back();

    std::string canonicalRequest = "PUT\n" + path + "\n" + queryString + "\n"
                                  + "host:" + host + "\n\nhost\nUNSIGNED-PAYLOAD";
    std::string stringToSign = "AWS4-HMAC-SHA256\n" + timestamp + "\n" + scope + "\n" + sha256(canonicalRequest);

    std::string kDate = hmacSha256("AWS4" + secretKey_, dateStamp);
    std::string kRegion = hmacSha256(kDate, region_);
    std::string kService = hmacSha256(kRegion, "s3");
    std::string kSigning = hmacSha256(kService, "aws4_request");
    std::string signature = hmacSha256(kSigning, stringToSign);

    std::ostringstream sigHex;
    for (unsigned char c : signature)
        sigHex << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(c);

    return endpoint_ + path + "?" + queryString + "&X-Amz-Signature=" + sigHex.str();
}

std::optional<S3Object> S3StorageClient::headObject(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!curl_) return std::nullopt;
    curl_easy_reset(curl_);

    std::string path = "/" + bucket_ + "/" + key;
    std::string url = endpoint_ + path;
    std::string timestamp = getTimestamp();
    std::string payloadHash = sha256("");
    std::string auth = signV4("HEAD", path, "", payloadHash, "application/octet-stream", timestamp);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: " + auth).c_str());
    headers = curl_slist_append(headers, ("x-amz-date: " + timestamp).c_str());
    headers = curl_slist_append(headers, ("x-amz-content-sha256: " + payloadHash).c_str());

    std::string headerData;
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_HEADERDATA, &headerData);

    CURLcode res = curl_easy_perform(curl_);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) return std::nullopt;

    long httpCode = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &httpCode);
    if (httpCode != 200) return std::nullopt;

    double cl = 0;
    curl_easy_getinfo(curl_, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &cl);

    S3Object obj;
    obj.key = key;
    obj.size = static_cast<size_t>(cl);
    return obj;
}

std::vector<S3Object> S3StorageClient::listObjects(const std::string& prefix, int maxKeys) {
    // Simplified - in production use ListObjectsV2 XML parsing
    (void)prefix;
    (void)maxKeys;
    return {};
}

bool S3StorageClient::enableVersioning() {
    // PUT /{bucket}?versioning with XML body
    std::lock_guard<std::mutex> lock(mtx_);
    if (!curl_) return false;
    curl_easy_reset(curl_);

    std::string path = "/" + bucket_ + "/";
    std::string url = endpoint_ + path + "?versioning";
    std::string body = "<VersioningConfiguration><Status>Enabled</Status></VersioningConfiguration>";
    std::string timestamp = getTimestamp();
    std::string payloadHash = sha256(body);
    std::string auth = signV4("PUT", path, "versioning", payloadHash, "application/xml", timestamp);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: " + auth).c_str());
    headers = curl_slist_append(headers, ("x-amz-date: " + timestamp).c_str());
    headers = curl_slist_append(headers, ("x-amz-content-sha256: " + payloadHash).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/xml");

    std::string response;
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl_);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) return false;

    long httpCode = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &httpCode);
    return httpCode >= 200 && httpCode < 300;
}

std::vector<nlohmann::json> S3StorageClient::listVersions(const std::string& key) {
    (void)key;
    // In production: GET /{bucket}?versions&prefix={key} and parse XML
    return {};
}

bool S3StorageClient::bucketExists() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!curl_) return false;
    curl_easy_reset(curl_);

    std::string path = "/" + bucket_ + "/";
    std::string url = endpoint_ + path;
    std::string timestamp = getTimestamp();
    std::string payloadHash = sha256("");
    std::string auth = signV4("HEAD", path, "", payloadHash, "application/octet-stream", timestamp);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: " + auth).c_str());
    headers = curl_slist_append(headers, ("x-amz-date: " + timestamp).c_str());
    headers = curl_slist_append(headers, ("x-amz-content-sha256: " + payloadHash).c_str());

    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl_);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) return false;

    long httpCode = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &httpCode);
    return httpCode == 200;
}
