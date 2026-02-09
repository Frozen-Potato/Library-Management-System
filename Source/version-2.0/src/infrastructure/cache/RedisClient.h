#pragma once
#include <string>
#include <optional>
#include <mutex>
#include <vector>
#include <functional>
#include <hiredis/hiredis.h>
#include <nlohmann/json.hpp>

class RedisClient {
public:
    RedisClient(const std::string& host, int port, const std::string& password = "");
    ~RedisClient();

    // Basic key-value operations
    bool set(const std::string& key, const std::string& value, int ttlSeconds = 0);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    bool expire(const std::string& key, int seconds);

    // JSON convenience methods
    bool setJson(const std::string& key, const nlohmann::json& value, int ttlSeconds = 0);
    std::optional<nlohmann::json> getJson(const std::string& key);

    // Session management
    bool setSession(const std::string& sessionId, const nlohmann::json& data, int ttlSeconds = 3600);
    std::optional<nlohmann::json> getSession(const std::string& sessionId);
    bool destroySession(const std::string& sessionId);

    // Cache invalidation patterns
    bool invalidateByPrefix(const std::string& prefix);

    // Health check
    bool ping();

private:
    redisContext* connect();
    redisReply* execute(const std::string& cmd);
    redisReply* execute(const std::vector<std::string>& args);

    std::string host_;
    int port_;
    std::string password_;
    redisContext* ctx_;
    std::mutex mtx_;
};
