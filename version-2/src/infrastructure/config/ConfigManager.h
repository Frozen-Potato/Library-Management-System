#pragma once
#include <string>

struct Config {
    std::string postgresUri;
    std::string mongoUri;
    std::string mongoDb;
    std::string mongoCollection;
    int queueIntervalSec = 2;
};

class ConfigManager {
public:
    static Config load();
};
