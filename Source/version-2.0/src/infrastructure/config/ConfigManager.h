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

    // Redis
    std::string redisHost;
    int redisPort;
    std::string redisPassword;

    // S3 / MinIO
    std::string s3Endpoint;
    std::string s3AccessKey;
    std::string s3SecretKey;
    std::string s3Bucket;
    std::string s3Region;

    // Kafka
    std::string kafkaBrokers;

    std::string appEnv;
    std::string logLevel;
};

class ConfigManager {
public:
    static Config load();
};
