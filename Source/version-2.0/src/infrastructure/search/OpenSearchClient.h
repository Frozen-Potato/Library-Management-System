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

    // Fuzzy full-text search with configurable fuzziness
    std::vector<nlohmann::json> fuzzySearch(const std::string& query,
                                            const std::string& fuzziness = "AUTO",
                                            int from = 0, int size = 10);

    // Auto-suggest / completion
    std::vector<std::string> autoSuggest(const std::string& prefix, int maxResults = 5);

    // Setup index with completion mapping (call once on startup)
    bool createIndexWithMapping();

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
