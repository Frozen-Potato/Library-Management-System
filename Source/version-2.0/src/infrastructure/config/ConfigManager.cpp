#include "src/infrastructure/config/ConfigManager.h"
#include "src/infrastructure/config/EnvLoader.h"

Config ConfigManager::load() {
    EnvLoader::load(".env");
    Config c;
    c.postgresUri = EnvLoader::get("POSTGRES_URI", "postgresql://postgres:password@postgres:5432/librarydb");
    c.mongoUri = EnvLoader::get("MONGO_URI", "mongodb://root:password@mongo:27017");
    c.mongoDb = EnvLoader::get("MONGO_DB", "library_logs");
    c.mongoCollection = EnvLoader::get("MONGO_COLLECTION", "audit_events");
    c.queueIntervalSec = std::stoi(EnvLoader::get("QUEUE_INTERVAL", "2"));
    c.restHost = EnvLoader::get("REST_HOST", "0.0.0.0");
    c.restPort = std::stoi(EnvLoader::get("REST_PORT", "8080"));
    c.grpcHost = EnvLoader::get("GRPC_HOST", "0.0.0.0");
    c.grpcPort = std::stoi(EnvLoader::get("GRPC_PORT", "50051"));
    c.jwtSecret = EnvLoader::get("JWT_SECRET", "super_secret_jwt_key");
    c.jwtExpirationMinutes = std::stoi(EnvLoader::get("JWT_EXPIRATION_MINUTES", "6000"));
    return c;
}
