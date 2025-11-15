#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class OpenSearchClient {
public:
    explicit OpenSearchClient(const std::string& baseUrl);

    bool indexMedia(int id, const std::string& title,
                    const std::string& author, const std::string& category);
    bool deleteMedia(int id);
    std::vector<nlohmann::json> searchMedia(const std::string& query) const;

private:
    std::string baseUrl_;
    bool sendRequest(const std::string& endpoint,
                     const std::string& method,
                     const std::string& body,
                     std::string& response) const;
};
