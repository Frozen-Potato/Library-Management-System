#pragma once
#include <string>
#include <unordered_map>

class EnvLoader {
public:
    static void load(const std::string& filepath);
    static std::string get(const std::string& key, const std::string& defaultValue = "");
    static std::string getOr(const std::string& key, const std::string& defaultValue) {
        return get(key, defaultValue);
    }

private:
    static std::unordered_map<std::string, std::string> env_;
};
