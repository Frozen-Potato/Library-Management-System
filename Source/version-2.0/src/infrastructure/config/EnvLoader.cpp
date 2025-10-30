#include "src/infrastructure/config/EnvLoader.h"
#include <fstream>
#include <sstream>
#include <algorithm>

std::unordered_map<std::string, std::string> EnvLoader::env_;

void EnvLoader::load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        key.erase(remove_if(key.begin(), key.end(), ::isspace), key.end());
        value.erase(remove_if(value.begin(), value.end(), ::isspace), value.end());
        env_[key] = value;
    }
}

std::string EnvLoader::get(const std::string& key, const std::string& defaultValue) {
    auto it = env_.find(key);
    return (it != env_.end()) ? it->second : defaultValue;
}
