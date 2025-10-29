#pragma once
#include <thread>
#include <atomic>
#include <memory>
#include "src/infrastructure/queue/PersistentQueue.h"
#include "src/data/MongoAdapter.h"

class QueueWorker {
public:
    QueueWorker(std::shared_ptr<PersistentQueue> queue,
                std::shared_ptr<MongoAdapter> mongo);

    void start();
    void stop();

private:
    void processBatch();

    std::shared_ptr<PersistentQueue> queue_;
    std::shared_ptr<MongoAdapter> mongo_;
    std::atomic<bool> stopFlag_;
    std::thread workerThread_;
};
