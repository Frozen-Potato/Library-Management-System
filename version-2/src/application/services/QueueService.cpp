#include "QueueService.h"
#include "pqxx/pqxx"

class PgQueueService : public QueueService {
public:
    explicit PgQueueService(std::shared_ptr<pqxx::connection> conn)
        : conn_(std::move(conn)) {}

    void enqueue(const std::string& taskType,
                 const nlohmann::json& payload) override {
        pqxx::work txn(*conn_);
        txn.exec_params(
            "INSERT INTO task_queue (task_type, payload, status) VALUES ($1, $2, 'PENDING');",
            taskType, payload.dump()
        );
        txn.commit();
    }

private:
    std::shared_ptr<pqxx::connection> conn_;
};
