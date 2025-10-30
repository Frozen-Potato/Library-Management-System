#include "PgQueueService.h"
#include "pqxx/pqxx"

PgQueueService::PgQueueService(std::shared_ptr<pqxx::connection> conn)
        : conn_(std::move(conn)) {}

void PgQueueService::enqueue(const std::string& taskType,
                const nlohmann::json& payload) {
    pqxx::work txn(*conn_);
    txn.exec(
        "INSERT INTO task_queue (task_type, payload, status) VALUES ($1, $2, 'PENDING');",
        pqxx::params{taskType, payload.dump()}
    );
    txn.commit();
}

