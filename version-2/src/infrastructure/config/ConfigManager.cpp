#include "src/infrastructure/config/ConfigManager.h"
#include "src/infrastructure/config/EnvLoader.h"

Config ConfigManager::load() {
    EnvLoader::load(".env");
    Config c;
    c.postgresUri = EnvLoader::get("POSTGRES_URI", "postgresql://postgres:postgres@localhost/library");
    c.mongoUri = EnvLoader::get("MONGO_URI", "mongodb://localhost:27017");
    c.mongoDb = EnvLoader::get("MONGO_DB", "library_logs");
    c.mongoCollection = EnvLoader::get("MONGO_COLLECTION", "audit_events");
    c.queueIntervalSec = std::stoi(EnvLoader::get("QUEUE_INTERVAL", "2"));
    return c;
}
