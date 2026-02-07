#pragma once
#include <string>

struct Config {
    std::string postgresUri;
    std::string mongoUri;
    std::string mongoDb;
    std::string mongoCollection;
    int queueIntervalSec;

    std::string restHost;
    int restPort;
    std::string grpcHost;
    int grpcPort;

    std::string jwtSecret;
    int jwtExpirationMinutes;

    std::string opensearchUrl;

    std::string appEnv;
    std::string logLevel;
};

class ConfigManager {
public:
    static Config load();
};
