#pragma once
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <bsoncxx/json.hpp>

#include "mongocxx/client.hpp"
#include "mongocxx/instance.hpp"
#include "src/data/MongoAdapter.h"
#include "src/utils/Exceptions.h"


class MongoAdapter {
public:
    explicit MongoAdapter(const std::string& uri, const std::string& dbName);

    void insertLog(const nlohmann::json& log);
    std::vector<nlohmann::json> fetchRecentLogs(int limit = 50);
    std::vector<nlohmann::json> fetchLogsByLevel(const std::string& level);

private:
    mongocxx::client client_;
    mongocxx::database db_;
};
