#include <iostream>
#include <thread>
#include <csignal>
#include <memory>
#include <atomic>

#include <crow.h>
#include <grpcpp/grpcpp.h>

#include "src/infrastructure/config/ConfigManager.h"
#include "src/infrastructure/db/PostgresPool.h"
#include "src/infrastructure/db/MongoConnection.h"
#include "src/infrastructure/queue/QueueWorker.h"
#include "src/infrastructure/jwt/JwtHelper.h"
#include "src/api/middleware/JwtMiddleware.h"
#include "src/api/middleware/PermissionMiddleware.h"

#include "src/application/services/LibraryService.h"
#include "src/application/services/PgQueueService.h"
#include "src/data/PostgresAdapter.h"
#include "src/data/MongoAdapter.h"

#include "src/api/controllers/MediaController.h"
#include "src/api/controllers/BorrowController.h"
#include "src/api/controllers/ReturnController.h"
#include "src/api/controllers/UserController.h"
#include "src/api/controllers/LoginController.h"
#include "src/api/grpc/LogServiceServer.h"

// Global flag for graceful shutdown
std::atomic<bool> running{true};

void handleSignal(int signal) {
    std::cout << "\n[System] Caught signal " << signal << ", shutting down..." << std::endl;
    running = false;
}

int main() {
    mongocxx::instance instance{};
    try {

        // Load configuration

        Config config = ConfigManager::load();


        // Initialize core infrastructure

        // PostgreSQL connection pool
        auto pgPool = std::make_shared<PostgresPool>();
        pgPool->initialize(config.postgresUri); 
        auto pgConn = pgPool->acquire();

        // MongoDB connection
        auto mongoAdapter = std::make_shared<MongoAdapter>(config.mongoUri, config.mongoDb);

        // Queue + Worker
        auto queueService = std::make_shared<PgQueueService>(pgConn);
        auto persistentQueue = std::make_shared<PersistentQueue>(pgConn);
        auto queueWorker = std::make_shared<QueueWorker>(persistentQueue, mongoAdapter);

        std::thread workerThread([&]() {
            std::cout << "[QueueWorker] Started background thread.\n";
            queueWorker->start();
        });

        // JWT setup

        auto jwtHelper = std::make_shared<JwtHelper>(
            config.jwtSecret,
            "library_system",
            config.jwtExpirationMinutes
        );

        crow::App<JwtMiddleware, PermissionMiddleware> app(jwtHelper);


        // Initialize application services

        auto dbAdapter = std::make_shared<PostgresAdapter>(pgConn);
        auto userService = std::make_shared<UserService>(dbAdapter);
        auto authService = std::make_shared<AuthService>(dbAdapter, jwtHelper);
        auto libraryService = std::make_shared<LibraryService>(dbAdapter, queueService);

        // Register controllers

        auto mediaController  = std::make_shared<MediaController>(libraryService);
        auto borrowController = std::make_shared<BorrowController>(libraryService);
        auto returnController = std::make_shared<ReturnController>(libraryService);
        auto userController  = std::make_shared<UserController>(userService);
        auto loginController = std::make_shared<LoginController>(authService);


        mediaController->registerRoutes(app);
        borrowController->registerRoutes(app);
        returnController->registerRoutes(app);
        userController->registerRoutes(app);
        loginController->registerRoutes(app);

        std::cout << "[REST] Routes registered successfully.\n";


        // Start Crow REST server

        std::thread restThread([&]() {
            std::cout << "[REST] Server running on port " << config.restPort << "...\n";
            app.port(config.restPort).multithreaded().run();
        });


        // Start gRPC LogService server

        std::thread grpcThread([&]() {
            std::string serverAddr = config.grpcHost + ":" + std::to_string(config.grpcPort);
            std::cout << "[gRPC] LogService running on " << serverAddr << "...\n";

            LogServiceServer logService(queueService, mongoAdapter);

            grpc::ServerBuilder builder;
            builder.AddListeningPort(serverAddr, grpc::InsecureServerCredentials());
            builder.RegisterService(&logService);
            std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
            server->Wait();
        });


        // Handle shutdown signals

        std::signal(SIGINT, handleSignal);
        std::signal(SIGTERM, handleSignal);

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "\n[System] Shutting down..." << std::endl;
        app.stop();
        queueWorker->stop();

        workerThread.join();
        restThread.join();
        grpcThread.detach(); // gRPC cleans up internally

        std::cout << "[System] Shutdown complete. Goodbye!" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "[Fatal] " << e.what() << std::endl;
        return 1;
    }
}
