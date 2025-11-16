#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <optional>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

class OpenSearchClient {
public:
    OpenSearchClient(const std::string& baseUrl,
                     const std::optional<std::string>& apiKey = std::nullopt);

    ~OpenSearchClient();

    bool indexMedia(int id,
                    const std::string& title,
                    const std::string& author,
                    const std::string& category);

    bool deleteMedia(int id);

    std::vector<nlohmann::json> searchMedia(const std::string& query);

    // Bulk indexing
    bool bulkIndex(const std::vector<nlohmann::json>& docs);

private:
    bool sendRequest(const std::string& endpoint,
                     const std::string& method,
                     const std::string& body,
                     std::string& response);

    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);

private:
    std::string baseUrl_;
    std::optional<std::string> apiKey_;
    CURL* curl_;
    std::mutex curlMutex_;
};
