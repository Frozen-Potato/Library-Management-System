#include "OpenSearchClient.h"
#include <iostream>

using json = nlohmann::json;

OpenSearchClient::OpenSearchClient(const std::string& baseUrl,
                                   const std::optional<std::string>& apiKey)
    : baseUrl_(baseUrl), apiKey_(apiKey) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_ = curl_easy_init();
}

OpenSearchClient::~OpenSearchClient() {
    if (curl_) curl_easy_cleanup(curl_);
    curl_global_cleanup();
}

size_t OpenSearchClient::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realSize = size * nmemb;
    reinterpret_cast<std::string*>(userp)->append((char*)contents, realSize);
    return realSize;
}

bool OpenSearchClient::sendRequest(const std::string& endpoint,
                                   const std::string& method,
                                   const std::string& body,
                                   std::string& response) {
    std::lock_guard<std::mutex> lock(curlMutex_);

    if (!curl_) return false;
    curl_easy_reset(curl_);

    std::string url = baseUrl_ + endpoint;

    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, method.c_str());

    // Timeouts
    curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT_MS, 3000);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, 5000);

    // Response handler
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

    // Body
    if (method == "POST" || method == "PUT") {
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, body.size());
    }

    // Headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    if (apiKey_.has_value()) {
        std::string authHeader = "Authorization: ApiKey " + apiKey_.value();
        headers = curl_slist_append(headers, authHeader.c_str());
    }

    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

    // Perform
    CURLcode res = curl_easy_perform(curl_);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        std::cerr << "[OpenSearch] CURL error: " << curl_easy_strerror(res) << "\n";
        return false;
    }

    // Check status
    long status = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &status);

    if (status < 200 || status >= 300) {
        std::cerr << "[OpenSearch] HTTP error: " << status << "\n"
                  << "Response: " << response << "\n";
        return false;
    }

    return true;
}

bool OpenSearchClient::indexMedia(int id,
                                  const std::string& title,
                                  const std::string& author,
                                  const std::string& category) {
    json body = {
        {"id", id},
        {"title", title},
        {"author", author},
        {"category", category}
    };

    std::string resp;
    return sendRequest("/media/_doc/" + std::to_string(id),
                       "PUT",
                       body.dump(),
                       resp);
}

bool OpenSearchClient::deleteMedia(int id) {
    std::string resp;
    return sendRequest("/media/_doc/" + std::to_string(id),
                       "DELETE",
                       "",
                       resp);
}

std::vector<json> OpenSearchClient::searchMedia(const std::string& query) {
    json q = {
        {"query", {
            {"multi_match", {
                {"query", query},
                {"fields", {"title", "author", "category"}}
            }}
        }}
    };

    std::string resp;
    if (!sendRequest("/media/_search", "POST", q.dump(), resp))
        return {};

    json parsed = json::parse(resp, nullptr, false);
    if (parsed.is_discarded())
        return {};

    std::vector<json> results;

    if (parsed.contains("hits") && parsed["hits"].contains("hits")) {
        for (auto& hit : parsed["hits"]["hits"]) {
            if (hit.contains("_source"))
                results.push_back(hit["_source"]);
        }
    }

    return results;
}

std::vector<json> OpenSearchClient::fuzzySearch(const std::string& query,
                                                const std::string& fuzziness,
                                                int from, int size) {
    json q = {
        {"from", from},
        {"size", size},
        {"query", {
            {"bool", {
                {"should", json::array({
                    {{"multi_match", {
                        {"query", query},
                        {"fields", {"title^3", "author^2", "category"}},
                        {"fuzziness", fuzziness},
                        {"prefix_length", 1},
                        {"max_expansions", 50}
                    }}},
                    {{"multi_match", {
                        {"query", query},
                        {"fields", {"title^3", "author^2", "category"}},
                        {"type", "phrase_prefix"}
                    }}}
                })},
                {"minimum_should_match", 1}
            }}
        }},
        {"highlight", {
            {"fields", {
                {"title", json::object()},
                {"author", json::object()}
            }}
        }}
    };

    std::string resp;
    if (!sendRequest("/media/_search", "POST", q.dump(), resp))
        return {};

    json parsed = json::parse(resp, nullptr, false);
    if (parsed.is_discarded())
        return {};

    std::vector<json> results;
    if (parsed.contains("hits") && parsed["hits"].contains("hits")) {
        for (auto& hit : parsed["hits"]["hits"]) {
            json result;
            if (hit.contains("_source"))
                result = hit["_source"];
            if (hit.contains("_score"))
                result["_score"] = hit["_score"];
            if (hit.contains("highlight"))
                result["_highlight"] = hit["highlight"];
            results.push_back(result);
        }
    }
    return results;
}

std::vector<std::string> OpenSearchClient::autoSuggest(const std::string& prefix, int maxResults) {
    json q = {
        {"suggest", {
            {"media-suggest", {
                {"prefix", prefix},
                {"completion", {
                    {"field", "suggest"},
                    {"size", maxResults},
                    {"fuzzy", {
                        {"fuzziness", "AUTO"}
                    }}
                }}
            }}
        }}
    };

    std::string resp;
    if (!sendRequest("/media/_search", "POST", q.dump(), resp))
        return {};

    json parsed = json::parse(resp, nullptr, false);
    if (parsed.is_discarded())
        return {};

    std::vector<std::string> suggestions;
    if (parsed.contains("suggest") && parsed["suggest"].contains("media-suggest")) {
        for (auto& entry : parsed["suggest"]["media-suggest"]) {
            if (entry.contains("options")) {
                for (auto& opt : entry["options"]) {
                    if (opt.contains("text"))
                        suggestions.push_back(opt["text"].get<std::string>());
                }
            }
        }
    }
    return suggestions;
}

bool OpenSearchClient::createIndexWithMapping() {
    json mapping = {
        {"settings", {
            {"number_of_shards", 1},
            {"number_of_replicas", 1},
            {"analysis", {
                {"analyzer", {
                    {"autocomplete_analyzer", {
                        {"type", "custom"},
                        {"tokenizer", "standard"},
                        {"filter", {"lowercase", "edge_ngram_filter"}}
                    }}
                }},
                {"filter", {
                    {"edge_ngram_filter", {
                        {"type", "edge_ngram"},
                        {"min_gram", 2},
                        {"max_gram", 20}
                    }}
                }}
            }}
        }},
        {"mappings", {
            {"properties", {
                {"id", {{"type", "integer"}}},
                {"title", {
                    {"type", "text"},
                    {"analyzer", "standard"},
                    {"fields", {
                        {"autocomplete", {
                            {"type", "text"},
                            {"analyzer", "autocomplete_analyzer"},
                            {"search_analyzer", "standard"}
                        }}
                    }}
                }},
                {"author", {{"type", "text"}}},
                {"category", {{"type", "keyword"}}},
                {"suggest", {
                    {"type", "completion"},
                    {"analyzer", "simple"}
                }}
            }}
        }}
    };

    std::string resp;
    return sendRequest("/media", "PUT", mapping.dump(), resp);
}

bool OpenSearchClient::bulkIndex(const std::vector<json>& docs) {
    std::string body;

    for (const auto& d : docs) {
        body += R"({"index":{"_index":"media","_id":")" 
             + std::to_string(d["id"].get<int>()) + R"("}})" "\n";
        body += d.dump() + "\n";
    }

    std::string resp;
    return sendRequest("/_bulk", "POST", body, resp);
}
