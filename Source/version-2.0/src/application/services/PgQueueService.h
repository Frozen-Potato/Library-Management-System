#pragma once
#include "QueueService.h"
#include <pqxx/pqxx>

class PgQueueService : public QueueService {
public:
    explicit PgQueueService(std::shared_ptr<pqxx::connection> conn);
    void enqueue(const std::string& taskType, const nlohmann::json& payload) override;

private:
    std::shared_ptr<pqxx::connection> conn_;
};
