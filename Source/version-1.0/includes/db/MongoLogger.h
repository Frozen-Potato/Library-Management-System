#ifndef MONGO_LOGGER_H
#define MONGO_LOGGER_H

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <iostream>
#include <chrono>
#include <ctime>
#include "EnvLoader.h"

class MongoLogger {
private:
    static inline std::unique_ptr<mongocxx::instance> instance = nullptr;
    mongocxx::client client;
    mongocxx::database db;

public:
    explicit MongoLogger(const std::string& mongoUrl)
        : client(mongocxx::uri{mongoUrl}),
          db(client["librarylogs"]) {
        std::cout << "[Mongo] Connected to " << mongoUrl << "\n";
    }

    void logEvent(const std::string& level, const std::string& message, const std::string& user = "system") {
        auto coll = db["logs"];
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        bsoncxx::builder::basic::document doc{};
        doc.append(
            bsoncxx::builder::basic::kvp("timestamp", std::ctime(&now)),
            bsoncxx::builder::basic::kvp("level", level),
            bsoncxx::builder::basic::kvp("message", message),
            bsoncxx::builder::basic::kvp("user", user)
        );

        try {
            coll.insert_one(doc.extract());
            std::cout << "[Mongo] Logged: " << level << " - " << message << "\n";
        } catch (const std::exception& e) {
            std::cerr << "[Mongo] Log failed: " << e.what() << "\n";
        }
    }
};

#endif
