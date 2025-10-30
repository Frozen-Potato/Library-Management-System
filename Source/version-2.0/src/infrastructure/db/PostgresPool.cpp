#include "src/infrastructure/db/PostgresPool.h"
#include <iostream>
#include <stdexcept>

std::queue<std::shared_ptr<pqxx::connection>> PostgresPool::pool_;
std::mutex PostgresPool::mtx_;
std::string PostgresPool::uri_;
int PostgresPool::maxConnections_ = 5;
bool PostgresPool::initialized_ = false;

void PostgresPool::initialize(const std::string& uri, int maxConnections) {
    std::scoped_lock lock(mtx_);
    if (initialized_) return;
    uri_ = uri;
    maxConnections_ = maxConnections;

    try {
        for (int i = 0; i < maxConnections_; ++i) {
            auto conn = std::make_shared<pqxx::connection>(uri_);
            if (!conn->is_open())
                throw std::runtime_error("Failed to open PostgreSQL connection");
            pool_.push(conn);
        }
        initialized_ = true;
        std::cout << "[PostgresPool] Initialized with " << maxConnections_ << " connections.\n";
    } catch (const std::exception& e) {
        std::cerr << "[PostgresPool] Initialization failed: " << e.what() << std::endl;
        throw;
    }
}

std::shared_ptr<pqxx::connection> PostgresPool::acquire() {
    std::unique_lock<std::mutex> lock(mtx_);
    if (pool_.empty()) {
        // Fallback: create new connection temporarily
        std::cerr << "[PostgresPool] Pool exhausted, creating temporary connection.\n";
        return std::make_shared<pqxx::connection>(uri_);
    }
    auto conn = pool_.front();
    pool_.pop();
    return conn;
}

void PostgresPool::release(std::shared_ptr<pqxx::connection> conn) {
    std::scoped_lock lock(mtx_);
    if (!conn || !conn->is_open()) return;
    pool_.push(std::move(conn));
}
