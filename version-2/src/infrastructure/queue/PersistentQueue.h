#pragma once
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <optional>
#include <nlohmann/json.hpp>
#include "pqxx/pqxx"

struct Task {
    long id;
    std::string type;
    nlohmann::json payload;
    std::string status;
};

class PersistentQueue {
public:
    explicit PersistentQueue(std::shared_ptr<pqxx::connection> conn);

    void enqueue(const std::string& type, const nlohmann::json& payload);
    std::optional<Task> dequeueOne();
    std::vector<Task> dequeueBatch(int limit);
    void markProcessed(long taskId);

private:
    std::shared_ptr<pqxx::connection> conn_;
    std::mutex mtx_;
};
