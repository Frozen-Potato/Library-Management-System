#pragma once
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <pqxx/pqxx>

// A simple lightweight PostgreSQL connection pool for C++17
class PostgresPool {
public:
    // Create singleton pool instance (URI, maxConnections)
    static void initialize(const std::string& uri, int maxConnections = 5);

    // Acquire a connection (returns shared_ptr)
    static std::shared_ptr<pqxx::connection> acquire();

    // Return connection to pool (optional, auto if shared_ptr refcount=0)
    static void release(std::shared_ptr<pqxx::connection> conn);

private:
    static std::queue<std::shared_ptr<pqxx::connection>> pool_;
    static std::mutex mtx_;
    static std::string uri_;
    static int maxConnections_;
    static bool initialized_;
};
