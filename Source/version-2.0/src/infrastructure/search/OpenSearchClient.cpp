#include "OpenSearchClient.h"
#include <curl/curl.h>
#include <iostream>

using json = nlohmann::json;

OpenSearchClient::OpenSearchClient(const std::string& baseUrl)
    : baseUrl_(baseUrl) {}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool OpenSearchClient::sendRequest(const std::string& endpoint,
                                   const std::string& method,
                                   const std::string& body,
                                   std::string& response) const {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    std::string url = baseUrl_ + endpoint;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    if (method == "POST" || method == "PUT")
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return res == CURLE_OK;
}

bool OpenSearchClient::indexMedia(int id, const std::string& title,
                                  const std::string& author, const std::string& category) {
    json body = {
        {"id", id},
        {"title", title},
        {"author", author},
        {"category", category}
    };
    std::string resp;
    return sendRequest("/media/_doc/" + std::to_string(id), "PUT", body.dump(), resp);
}

bool OpenSearchClient::deleteMedia(int id) {
    std::string resp;
    return sendRequest("/media/_doc/" + std::to_string(id), "DELETE", "", resp);
}

std::vector<json> OpenSearchClient::searchMedia(const std::string& query) const {
    json q = {
        {"query", {{"multi_match", {{"query", query}, {"fields", {"title", "author", "category"}}}}}}
    };
    std::string resp;
    if (!sendRequest("/media/_search", "POST", q.dump(), resp)) return {};

    json parsed = json::parse(resp, nullptr, false);
    std::vector<json> results;
    if (!parsed.is_null() && parsed.contains("hits")) {
        for (auto& h : parsed["hits"]["hits"])
            results.push_back(h["_source"]);
    }
    return results;
}
