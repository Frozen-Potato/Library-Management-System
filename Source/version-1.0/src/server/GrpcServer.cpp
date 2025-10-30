#include "GrpcServer.h"
#include "log_service.grpc.pb.h"
#include "MongoLogger.h"
#include <grpcpp/grpcpp.h>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using library::LogRequest;
using library::LogResponse;
using library::LogEntry;
using library::LogService;

class LogServiceImpl final : public LogService::Service {
public:
    LogServiceImpl(const std::string& uri) : client(mongocxx::uri{uri}), db(client["librarylogs"]) {}

    Status GetLogs(ServerContext*, const LogRequest* request, LogResponse* reply) override {
        auto coll = db["logs"];
        mongocxx::options::find opts{};
        if (!request->level().empty()) {
            bsoncxx::builder::basic::document filter;
            filter.append(bsoncxx::builder::basic::kvp("level", request->level()));
            auto cursor = coll.find(filter.view());
            fillResponse(cursor, reply);
        } else {
            auto cursor = coll.find({});
            fillResponse(cursor, reply);
        }
        return Status::OK;
    }

private:
    mongocxx::client client;
    mongocxx::database db;

    void fillResponse(mongocxx::cursor& cursor, LogResponse* reply) {
        for (auto&& doc : cursor) {
            LogEntry* entry = reply->add_logs();
            entry->set_timestamp(std::string(doc["timestamp"].get_string().value));
            entry->set_level(std::string(doc["level"].get_string().value));
            entry->set_message(std::string(doc["message"].get_string().value));
            entry->set_user(std::string(doc["user"].get_string().value));

        }
    }
};

void openGrpcServer() {
    EnvLoader env;
    MongoLogger logger(env.getMongoUrl());

    grpc::reflection::InitProtoReflectionServerBuilderPlugin(); 
    std::string address("0.0.0.0:50051");
    std::string mongo_uri = env.getMongoUrl();
    LogServiceImpl service(mongo_uri);

    ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "[gRPC] Server listening on " << address << std::endl;
    server->Wait();
}
