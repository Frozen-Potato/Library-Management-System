#include "src/api/grpc/LogServiceServer.h"
#include "src/utils/Exceptions.h"
#include <iostream>

using namespace library;

grpc::Status LogServiceServer::GetLogs(grpc::ServerContext* context,
                                       const LogRequest* request,
                                       LogResponse* response) {
    try {
        std::string level = request->level();

        if (level.empty()) {
            level = "ALL";
        }

        auto logs = mongoLogger_->fetchLogsByLevel(level);

        for (const auto& log : logs) {
            auto* entry = response->add_logs();

            entry->set_timestamp(log.value("timestamp", ""));
            entry->set_level(log.value("level", ""));
            entry->set_message(log.value("message", ""));
            entry->set_user(log.value("user", ""));
        }

        return grpc::Status::OK;

    } catch (const ValidationException& e) {
        std::cerr << "[LogServiceServer] Validation error: " << e.what() << std::endl;
        return grpc::Status(grpc::INVALID_ARGUMENT, e.what());

    } catch (const DatabaseException& e) {
        std::cerr << "[LogServiceServer] Database error: " << e.what() << std::endl;
        return grpc::Status(grpc::INTERNAL, e.what());

    } catch (const std::exception& e) {
        std::cerr << "[LogServiceServer] Unexpected error: " << e.what() << std::endl;
        return grpc::Status(grpc::UNKNOWN, "Unexpected error occurred");
    }
}
