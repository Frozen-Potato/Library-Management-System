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
#include "src/infrastructure/cache/RedisClient.h"
#include "src/infrastructure/storage/S3StorageClient.h"
#include "src/infrastructure/search/OpenSearchClient.h"
#include "src/infrastructure/messaging/KafkaProducer.h"
#include "src/infrastructure/messaging/KafkaConsumer.h"
#include "src/infrastructure/metrics/MetricsRegistry.h"
#include "src/api/middleware/JwtMiddleware.h"
#include "src/api/middleware/PermissionMiddleware.h"

#include "src/application/services/LibraryService.h"
#include "src/application/services/PgQueueService.h"
#include "src/application/services/DigitalMediaService.h"
#include "src/application/services/BatchImportService.h"
#include "src/data/PostgresAdapter.h"
#include "src/data/MongoAdapter.h"

#include "src/api/controllers/MediaController.h"
#include "src/api/controllers/BorrowController.h"
#include "src/api/controllers/ReturnController.h"
#include "src/api/controllers/UserController.h"
#include "src/api/controllers/LoginController.h"
#include "src/api/controllers/SearchController.h"
#include "src/api/controllers/DigitalMediaController.h"
#include "src/api/controllers/BatchImportController.h"
#include "src/api/controllers/MetricsController.h"
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

        // Redis cache
        auto redisClient = std::make_shared<RedisClient>(
            config.redisHost, config.redisPort, config.redisPassword);
        if (redisClient->ping()) {
            std::cout << "[Redis] Connected successfully.\n";
        } else {
            std::cerr << "[Redis] Connection failed, caching disabled.\n";
        }

        // S3 / MinIO storage
        auto s3Client = std::make_shared<S3StorageClient>(
            config.s3Endpoint, config.s3AccessKey, config.s3SecretKey,
            config.s3Bucket, config.s3Region);
        if (s3Client->bucketExists()) {
            std::cout << "[S3] Bucket '" << config.s3Bucket << "' ready.\n";
        } else {
            std::cerr << "[S3] Bucket not found. Digital media storage may not work.\n";
        }

        // OpenSearch
        auto searchClient = std::make_shared<OpenSearchClient>(config.opensearchUrl);
        searchClient->createIndexWithMapping();
        std::cout << "[OpenSearch] Index initialized.\n";

        // Kafka producer
        auto kafkaProducer = std::make_shared<KafkaProducer>(config.kafkaBrokers, "library-producer");
        std::cout << "[Kafka] Producer initialized.\n";

        // Kafka consumer for event processing
        auto kafkaConsumer = std::make_shared<KafkaConsumer>(
            config.kafkaBrokers, "library-consumers",
            std::vector<std::string>{"media.events", "user.events"});

        kafkaConsumer->start([&mongoAdapter](const KafkaMessage& msg) {
            try {
                auto payload = nlohmann::json::parse(msg.value);
                mongoAdapter->insertLog(payload);
            } catch (const std::exception& e) {
                std::cerr << "[Kafka] Consumer error: " << e.what() << std::endl;
            }
        });

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

        // Initialize Prometheus metrics
        auto& metrics = MetricsRegistry::instance();
        metrics.counter("http_requests_total", "Total HTTP requests");
        metrics.histogram("http_request_duration_seconds", "HTTP request latency");
        metrics.gauge("active_connections", "Currently active connections");

        // Initialize application services

        auto dbAdapter = std::make_shared<PostgresAdapter>(pgConn);
        auto userService = std::make_shared<UserService>(dbAdapter);
        auto authService = std::make_shared<AuthService>(dbAdapter, jwtHelper);
        auto libraryService = std::make_shared<LibraryService>(dbAdapter, queueService, searchClient);
        auto digitalMediaService = std::make_shared<DigitalMediaService>(
            dbAdapter, s3Client, kafkaProducer, redisClient);
        auto batchImportService = std::make_shared<BatchImportService>(
            dbAdapter, searchClient, kafkaProducer);

        // Register controllers

        auto mediaController       = std::make_shared<MediaController>(libraryService);
        auto borrowController      = std::make_shared<BorrowController>(libraryService);
        auto returnController      = std::make_shared<ReturnController>(libraryService);
        auto userController        = std::make_shared<UserController>(userService);
        auto loginController       = std::make_shared<LoginController>(authService);
        auto searchController      = std::make_shared<SearchController>(libraryService, searchClient);
        auto digitalMediaCtrl      = std::make_shared<DigitalMediaController>(digitalMediaService);
        auto batchImportCtrl       = std::make_shared<BatchImportController>(batchImportService);
        auto metricsController     = std::make_shared<MetricsController>();


        mediaController->registerRoutes(app);
        borrowController->registerRoutes(app);
        returnController->registerRoutes(app);
        userController->registerRoutes(app);
        loginController->registerRoutes(app);
        searchController->registerRoutes(app);
        digitalMediaCtrl->registerRoutes(app);
        batchImportCtrl->registerRoutes(app);
        metricsController->registerRoutes(app);

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
        kafkaConsumer->stop();
        kafkaProducer->flush(5000);

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
