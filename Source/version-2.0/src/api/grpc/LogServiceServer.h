#pragma once
#include "proto/log_service.grpc.pb.h"
#include "src/application/services/QueueService.h"
#include "src/data/MongoAdapter.h"
#include <memory>
#include <nlohmann/json.hpp>

class LogServiceServer final : public library::LogService::Service {
public:
    LogServiceServer(std::shared_ptr<QueueService> queue,
                     std::shared_ptr<MongoAdapter> mongo)
        : queueService_(std::move(queue)), mongoLogger_(std::move(mongo)) {}

    grpc::Status GetLogs(grpc::ServerContext* context,
                         const library::LogRequest* request,
                         library::LogResponse* response) override;

private:
    std::shared_ptr<QueueService> queueService_;
    std::shared_ptr<MongoAdapter> mongoLogger_;
};
