#include "RedisClient.h"
#include <iostream>
#include <stdexcept>

RedisClient::RedisClient(const std::string& host, int port, const std::string& password)
    : host_(host), port_(port), password_(password), ctx_(nullptr) {
    ctx_ = connect();
}

RedisClient::~RedisClient() {
    if (ctx_) redisFree(ctx_);
}

redisContext* RedisClient::connect() {
    struct timeval timeout = {2, 0};
    redisContext* c = redisConnectWithTimeout(host_.c_str(), port_, timeout);
    if (!c || c->err) {
        std::string err = c ? c->errstr : "allocation failed";
        if (c) redisFree(c);
        std::cerr << "[Redis] Connection failed: " << err << std::endl;
        return nullptr;
    }

    if (!password_.empty()) {
        auto* reply = static_cast<redisReply*>(
            redisCommand(c, "AUTH %s", password_.c_str()));
        if (!reply || reply->type == REDIS_REPLY_ERROR) {
            std::cerr << "[Redis] AUTH failed" << std::endl;
            if (reply) freeReplyObject(reply);
            redisFree(c);
            return nullptr;
        }
        freeReplyObject(reply);
    }

    return c;
}

redisReply* RedisClient::execute(const std::string& cmd) {
    if (!ctx_) {
        ctx_ = connect();
        if (!ctx_) return nullptr;
    }
    auto* reply = static_cast<redisReply*>(redisCommand(ctx_, cmd.c_str()));
    if (!reply && ctx_->err) {
        redisFree(ctx_);
        ctx_ = connect();
        if (ctx_)
            reply = static_cast<redisReply*>(redisCommand(ctx_, cmd.c_str()));
    }
    return reply;
}

redisReply* RedisClient::execute(const std::vector<std::string>& args) {
    if (!ctx_) {
        ctx_ = connect();
        if (!ctx_) return nullptr;
    }
    std::vector<const char*> argv;
    std::vector<size_t> argvlen;
    argv.reserve(args.size());
    argvlen.reserve(args.size());
    for (auto& a : args) {
        argv.push_back(a.c_str());
        argvlen.push_back(a.size());
    }
    auto* reply = static_cast<redisReply*>(
        redisCommandArgv(ctx_, static_cast<int>(args.size()), argv.data(), argvlen.data()));
    if (!reply && ctx_->err) {
        redisFree(ctx_);
        ctx_ = connect();
        if (ctx_)
            reply = static_cast<redisReply*>(
                redisCommandArgv(ctx_, static_cast<int>(args.size()), argv.data(), argvlen.data()));
    }
    return reply;
}

bool RedisClient::set(const std::string& key, const std::string& value, int ttlSeconds) {
    std::lock_guard<std::mutex> lock(mtx_);
    redisReply* reply;
    if (ttlSeconds > 0) {
        reply = execute({"SET", key, value, "EX", std::to_string(ttlSeconds)});
    } else {
        reply = execute({"SET", key, value});
    }
    bool ok = reply && reply->type == REDIS_REPLY_STATUS;
    if (reply) freeReplyObject(reply);
    return ok;
}

std::optional<std::string> RedisClient::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto* reply = execute({"GET", key});
    if (!reply) return std::nullopt;
    std::optional<std::string> result;
    if (reply->type == REDIS_REPLY_STRING)
        result = std::string(reply->str, reply->len);
    freeReplyObject(reply);
    return result;
}

bool RedisClient::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto* reply = execute({"DEL", key});
    bool ok = reply && reply->type == REDIS_REPLY_INTEGER && reply->integer > 0;
    if (reply) freeReplyObject(reply);
    return ok;
}

bool RedisClient::exists(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto* reply = execute({"EXISTS", key});
    bool ok = reply && reply->type == REDIS_REPLY_INTEGER && reply->integer > 0;
    if (reply) freeReplyObject(reply);
    return ok;
}

bool RedisClient::expire(const std::string& key, int seconds) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto* reply = execute({"EXPIRE", key, std::to_string(seconds)});
    bool ok = reply && reply->type == REDIS_REPLY_INTEGER && reply->integer == 1;
    if (reply) freeReplyObject(reply);
    return ok;
}

bool RedisClient::setJson(const std::string& key, const nlohmann::json& value, int ttlSeconds) {
    return set(key, value.dump(), ttlSeconds);
}

std::optional<nlohmann::json> RedisClient::getJson(const std::string& key) {
    auto val = get(key);
    if (!val.has_value()) return std::nullopt;
    auto parsed = nlohmann::json::parse(val.value(), nullptr, false);
    if (parsed.is_discarded()) return std::nullopt;
    return parsed;
}

bool RedisClient::setSession(const std::string& sessionId, const nlohmann::json& data, int ttlSeconds) {
    return setJson("session:" + sessionId, data, ttlSeconds);
}

std::optional<nlohmann::json> RedisClient::getSession(const std::string& sessionId) {
    return getJson("session:" + sessionId);
}

bool RedisClient::destroySession(const std::string& sessionId) {
    return del("session:" + sessionId);
}

bool RedisClient::invalidateByPrefix(const std::string& prefix) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto* reply = execute({"KEYS", prefix + "*"});
    if (!reply || reply->type != REDIS_REPLY_ARRAY) {
        if (reply) freeReplyObject(reply);
        return false;
    }
    for (size_t i = 0; i < reply->elements; ++i) {
        auto* delReply = execute({"DEL", std::string(reply->element[i]->str, reply->element[i]->len)});
        if (delReply) freeReplyObject(delReply);
    }
    freeReplyObject(reply);
    return true;
}

bool RedisClient::ping() {
    std::lock_guard<std::mutex> lock(mtx_);
    auto* reply = execute({"PING"});
    bool ok = reply && reply->type == REDIS_REPLY_STATUS
              && std::string(reply->str) == "PONG";
    if (reply) freeReplyObject(reply);
    return ok;
}
