#include "src/data/MongoAdapter.h"
#include "src/utils/Exceptions.h"
#include <iostream>

MongoAdapter::MongoAdapter(const std::string& uri, const std::string& dbName)
    : client_(mongocxx::uri{uri}), db_(client_[dbName]) {}

void MongoAdapter::insertLog(const nlohmann::json& log) {
    try {
        auto collection = db_["logs"];
        bsoncxx::document::value doc = bsoncxx::from_json(log.dump());
        collection.insert_one(doc.view());
    } catch (const std::exception& e) {
        throw DatabaseException(std::string("Mongo insert failed: ") + e.what());
    }
}

std::vector<nlohmann::json> MongoAdapter::fetchRecentLogs(int limit) {
    std::vector<nlohmann::json> results;
    try {
        auto collection = db_["logs"];
        mongocxx::options::find opts;
        opts.sort(bsoncxx::from_json(R"({"timestamp": -1})"));
        opts.limit(limit);

        auto cursor = collection.find({}, opts);
        for (auto&& doc : cursor) {
            results.push_back(nlohmann::json::parse(bsoncxx::to_json(doc)));
        }
    } catch (const std::exception& e) {
        throw DatabaseException(std::string("Mongo query failed: ") + e.what());
    }
    return results;
}

std::vector<nlohmann::json> MongoAdapter::fetchLogsByLevel(const std::string& level) {
    std::vector<nlohmann::json> results;
    try {
        auto collection = db_["logs"];
        mongocxx::options::find opts;
        opts.sort(bsoncxx::from_json(R"({"timestamp": -1})"));
        opts.limit(50);

        bsoncxx::builder::basic::document builder;
        if (level != "ALL") {
            builder.append(bsoncxx::builder::basic::kvp("level", level));
        }
        auto filter = builder.extract();

        auto cursor = collection.find(filter.view(), opts);
        for (auto&& doc : cursor) {
            results.push_back(nlohmann::json::parse(bsoncxx::to_json(doc)));
        }

    } catch (const std::exception& e) {
        throw DatabaseException(std::string("Mongo query failed: ") + e.what());
    }
    return results;
}
