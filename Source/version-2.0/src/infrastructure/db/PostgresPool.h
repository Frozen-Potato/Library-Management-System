#pragma once
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <pqxx/pqxx>

class PostgresPool {
public:

    static void initialize(const std::string& uri, int maxConnections = 5);

    static std::shared_ptr<pqxx::connection> acquire();

    static void release(std::shared_ptr<pqxx::connection> conn);

private:
    static std::queue<std::shared_ptr<pqxx::connection>> pool_;
    static std::mutex mtx_;
    static std::string uri_;
    static int maxConnections_;
    static bool initialized_;
};
