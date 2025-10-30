#include "src/infrastructure/queue/PersistentQueue.h"
#include <iostream>

PersistentQueue::PersistentQueue(std::shared_ptr<pqxx::connection> conn)
    : conn_(std::move(conn)) {}

void PersistentQueue::enqueue(const std::string& type, const nlohmann::json& payload) {
    std::scoped_lock lock(mtx_);
    try {
        pqxx::work txn(*conn_);
        txn.exec(
            "INSERT INTO task_queue (task_type, payload, status) VALUES ($1, $2, 'PENDING');",
            pqxx::params{type, payload.dump()}
        );
        txn.commit();
    } catch (const std::exception& e) {
        std::cerr << "[PersistentQueue] Enqueue failed: " << e.what() << std::endl;
    }
}

std::optional<Task> PersistentQueue::dequeueOne() {
    std::scoped_lock lock(mtx_);
    pqxx::work txn(*conn_);
    auto r = txn.exec(
        "UPDATE task_queue "
        "SET status = 'PROCESSING' "
        "WHERE id = (SELECT id FROM task_queue WHERE status = 'PENDING' ORDER BY created_at LIMIT 1 FOR UPDATE SKIP LOCKED) "
        "RETURNING id, task_type, payload, status;"
    );
    txn.commit();

    if (r.empty()) return std::nullopt;
    Task t;
    t.id = r[0]["id"].as<long>();
    t.type = r[0]["task_type"].as<std::string>();
    t.payload = nlohmann::json::parse(r[0]["payload"].as<std::string>());
    t.status = r[0]["status"].as<std::string>();
    return t;
}

std::vector<Task> PersistentQueue::dequeueBatch(int limit) {
    std::scoped_lock lock(mtx_);
    pqxx::work txn(*conn_);
    auto r = txn.exec(
        "UPDATE task_queue "
        "SET status = 'PROCESSING' "
        "WHERE id IN (SELECT id FROM task_queue WHERE status = 'PENDING' ORDER BY created_at LIMIT $1 FOR UPDATE SKIP LOCKED) "
        "RETURNING id, task_type, payload, status;",
        pqxx::params{limit}
    );
    txn.commit();

    std::vector<Task> batch;
    for (const auto& row : r) {
        Task t;
        t.id = row["id"].as<long>();
        t.type = row["task_type"].as<std::string>();
        t.payload = nlohmann::json::parse(row["payload"].as<std::string>());
        t.status = row["status"].as<std::string>();
        batch.push_back(t);
    }
    return batch;
}

void PersistentQueue::markProcessed(long taskId) {
    pqxx::work txn(*conn_);
    txn.exec("UPDATE task_queue SET status = 'DONE' WHERE id = $1;", pqxx::params{taskId});
    txn.commit();
}
