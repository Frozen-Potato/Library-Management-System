#include "src/infrastructure/queue/QueueWorker.h"
#include <chrono>
#include <iostream>

QueueWorker::QueueWorker(std::shared_ptr<PersistentQueue> queue,
                         std::shared_ptr<MongoAdapter> mongo)
    : queue_(std::move(queue)), mongo_(std::move(mongo)), stopFlag_(false) {}

void QueueWorker::start() {
    workerThread_ = std::thread([this]() {
        std::cout << "[QueueWorker] Started async worker thread." << std::endl;
        while (!stopFlag_) {
            auto tasks = queue_->dequeueBatch(10);
            if (!tasks.empty()) {
                std::vector<nlohmann::json> logs;
                for (auto& t : tasks) {
                    logs.push_back(t.payload);
                    queue_->markProcessed(t.id);
                }
                mongo_->insertLog(logs);
            }
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    });
}

void QueueWorker::stop() {
    stopFlag_ = true;
    if (workerThread_.joinable()) workerThread_.join();
    std::cout << "[QueueWorker] Worker stopped." << std::endl;
}
