#pragma once
#include <nlohmann/json.hpp>
#include <string>

inline nlohmann::json parseJsonSafe(const std::string& body) {
    try {
        auto json = nlohmann::json::parse(body);
        if (!json.is_object())
            throw std::runtime_error("Expected JSON object");
        return json;
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Invalid JSON: ") + e.what());
    }
}

inline std::string makeJsonError(const std::string& message, int code = 400) {
    nlohmann::json j = {{"error", message}, {"code", code}};
    return j.dump();
}

inline std::string makeJsonSuccess(const std::string& message) {
    nlohmann::json j = {{"status", "success"}, {"message", message}};
    return j.dump();
}
