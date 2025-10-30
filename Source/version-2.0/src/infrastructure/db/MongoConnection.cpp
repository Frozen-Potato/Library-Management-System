#include "src/infrastructure/db/MongoConnection.h"
#include <iostream>

std::unique_ptr<mongocxx::instance> MongoConnection::instance_ = nullptr;

std::shared_ptr<mongocxx::client> MongoConnection::create(const std::string& uri) {
    if (!instance_) {
        instance_ = std::make_unique<mongocxx::instance>();
        std::cout << "[MongoConnection] MongoDB driver initialized.\n";
    }

    try {
        auto client = std::make_shared<mongocxx::client>(mongocxx::uri{uri});
        std::cout << "[MongoConnection] Connected to MongoDB at: " << uri << std::endl;
        return client;
    } catch (const std::exception& e) {
        std::cerr << "[MongoConnection] Failed to connect to MongoDB: " << e.what() << std::endl;
        throw;
    }
}
