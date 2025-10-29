#include <iostream>
#include <thread>
#include <csignal>
#include <memory>

#include "src/infrastructure/config/EnvLoader.h"
#include "src/infrastructure/db/PostgresPool.h"
#include "src/infrastructure/db/MongoConnection.h"

#include "src/infrastructure/queue/QueueWorker.h"
#include "src/application/services/LibraryService.h"
#include "src/api/controllers/MediaController.h"
#include "src/api/controllers/BorrowController.h"
#include "src/api/controllers/ReturnController.h"
#include "src/api/controllers/UserController.h"
#include "src/api/controllers/LoginController.h"
#include "src/api/grpc/LogServiceServer.h"

#include <crow.h>
#include <grpcpp/grpcpp.h>

// Global stop flag for graceful shutdown
std::atomic<bool> running{true};

// Signal handler
void handleSignal(int signal) {
    std::cout << "\n[System] Caught signal " << signal << ", shutting down..." << std::endl;
    running = false;
}

int main() {
    try {
        // --- Load environment variables ---
        EnvLoader::load(".env");
        std::string pgConnStr = EnvLoader::get("PG_CONN");
        std::string mongoUri = EnvLoader::get("MONGO_URI");
        std::string mongoDb = EnvLoader::get("MONGO_DB");
        int restPort = std::stoi(EnvLoader::getOr("REST_PORT", "8080"));
        int grpcPort = std::stoi(EnvLoader::getOr("GRPC_PORT", "50051"));

        // --- Initialize core infrastructure ---
        auto pgPool = std::make_shared<PostgresPool>(pgConnStr);
        auto mongo = std::make_shared<MongoConnection>(mongoUri, mongoDb);
        auto queue = std::make_shared<PersistentQueue>("queue_buffer.db");
        auto queueWorker = std::make_shared<QueueWorker>(queue, mongo);

        // --- Start queue worker thread ---
        std::thread workerThread([&]() {
            std::cout << "[QueueWorker] Started background thread." << std::endl;
            queueWorker->start();
        });

        // --- Initialize LibraryService ---
        auto dbAdapter = std::make_shared<PostgresAdapter>(pgPool->acquire());
        auto queueService = std::make_shared<QueueService>(queue);
        auto libraryService = std::make_shared<LibraryService>(dbAdapter, queueService);

        // --- Setup Crow REST server ---
        crow::App<JwtMiddleware> app;
        

        auto mediaController   = std::make_shared<MediaController>(libraryService);
        auto borrowController  = std::make_shared<BorrowController>(libraryService);
        auto returnController  = std::make_shared<ReturnController>(libraryService);
        auto userController    = std::make_shared<UserController>(libraryService);
        auto loginController   = std::make_shared<LoginController>(libraryService);

        // Register all routes
        mediaController->registerRoutes(app);
        borrowController->registerRoutes(app);
        returnController->registerRoutes(app);
        userController->registerRoutes(app);
        loginController->registerRoutes(app);

        std::cout << "[REST] Routes registered successfully." << std::endl;

        std::thread restThread([&]() {
            std::cout << "[REST] Server running on port " << restPort << "..." << std::endl;
            app.port(restPort).multithreaded().run();
        });

        // --- Setup gRPC LogService ---
        std::thread grpcThread([&]() {
            std::string serverAddr = "0.0.0.0:" + std::to_string(grpcPort);
            std::cout << "[gRPC] LogService running on " << serverAddr << "..." << std::endl;

            auto queueServiceGrpc = std::make_shared<QueueService>(queue);
            auto mongoAdapter = std::make_shared<MongoAdapter>(mongoUri, mongoDb);
            LogServiceServer logService(queueServiceGrpc, mongoAdapter);

            grpc::ServerBuilder builder;
            builder.AddListeningPort(serverAddr, grpc::InsecureServerCredentials());
            builder.RegisterService(&logService);
            std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
            server->Wait();
        });

        // --- Graceful shutdown handling ---
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
        grpcThread.detach(); // gRPC cleans itself up internally

        std::cout << "[System] Shutdown complete. Goodbye!" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "[Fatal] " << e.what() << std::endl;
        return 1;
    }
}
