#include "RestServer.h"
#include "GrpcServer.h"
#include <thread>
#include <iostream>
#include "MongoLogger.h"
#include <mongocxx/instance.hpp>
#include "EnvLoader.h"

int main() {
    mongocxx::instance instance{};
    EnvLoader env;
    MongoLogger logger(env.getMongoUrl());
    logger.logEvent("INFO", "Library servers starting");
    std::cout << "[System] Starting combined Library servers...\n";

    std::thread rest_thread([]() { openRestServer(); });
    std::thread grpc_thread([]() { openGrpcServer(); });

    rest_thread.join();
    grpc_thread.join();
    return 0;
}