#pragma once
#include <memory>
#include <string>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

class MongoConnection {
public:
    static std::shared_ptr<mongocxx::client> create(const std::string& uri);

private:
    static std::unique_ptr<mongocxx::instance> instance_;
};

