# Library Management System

A scalable library management system built in C++20 for managing physical and digital media. Supports books, magazines, DVDs, audiobooks, and digital content (eBooks, PDFs, video) with role-based access control, full-text search, event-driven architecture, and object storage.

## Table of Contents

- [Architecture](#architecture)
- [Technology Stack](#technology-stack)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Configuration](#configuration)
- [Running the Application](#running-the-application)
- [API Reference](#api-reference)
- [Database Schema](#database-schema)
- [Sample Data](#sample-data)
- [Monitoring](#monitoring)
- [Project Structure](#project-structure)

## Architecture

The system follows Clean Architecture with four layers:

- **API Layer** -- REST controllers (Crow framework) and gRPC services handling HTTP/RPC requests
- **Application Layer** -- Business logic services (LibraryService, AuthService, DigitalMediaService, BatchImportService)
- **Domain Layer** -- Core entities (Media, User, BorrowRecord) with no external dependencies
- **Infrastructure Layer** -- Database adapters (PostgreSQL, MongoDB), Redis cache, S3 storage, Kafka messaging, OpenSearch, Prometheus metrics

External services:

```
Client --> REST API (port 8080) --> Application Services --> PostgreSQL (primary data)
                                                        --> MongoDB (audit logs)
                                                        --> Redis (caching / sessions)
                                                        --> OpenSearch (full-text search)
                                                        --> MinIO/S3 (digital media files)
                                                        --> Kafka (event streaming)
       --> gRPC API (port 50051) --> Log Service
```

## Technology Stack

| Component         | Technology                          |
|-------------------|-------------------------------------|
| Language          | C++20                               |
| Build System      | CMake 3.16+                         |
| Web Framework     | Crow (REST)                         |
| RPC Framework     | gRPC + Protocol Buffers             |
| Primary Database  | PostgreSQL 16                       |
| Document Store    | MongoDB 7                           |
| Cache             | Redis 7                             |
| Search Engine     | OpenSearch 2.11                     |
| Object Storage    | MinIO (S3-compatible)               |
| Message Broker    | Apache Kafka (Confluent 7.5)        |
| Monitoring        | Prometheus + Grafana                |
| Auth              | JWT (jwt-cpp) + bcrypt              |
| Serialization     | nlohmann/json                       |
| Containerization  | Docker + Docker Compose             |

## Prerequisites

- Docker Engine 20.10+
- Docker Compose v2+
- 8 GB RAM minimum (for all services)
- 10 GB free disk space

For local development without Docker:

- GCC 11+ or Clang 14+ (C++20 support)
- CMake 3.16+
- pkg-config
- Libraries: libpqxx, libmongocxx, libhiredis, librdkafka, libcurl, openssl, protobuf, gRPC, bcrypt

## Installation

### Using Docker Compose (recommended)

1. Clone the repository:

```bash
git clone https://github.com/Frozen-Potato/Library-Management-System.git
cd Library-Management-System/Source/version-2.0
```

2. Create a `.env` file (or use the existing one):

```bash
cp .env.example .env  # if available, otherwise see Configuration section below
```

3. Build and start all services:

```bash
cd docker
docker compose up --build -d
```

This starts PostgreSQL, MongoDB, Redis, OpenSearch, MinIO, Kafka, Prometheus, Grafana, and the application server.

4. Wait for all services to become healthy:

```bash
docker compose ps
```

All services should show `healthy` status. The backend depends on database services and will start after they pass health checks.

5. Verify the application is running:

```bash
curl http://localhost:8080/health
```

Expected response:
```json
{"service":"library-management-system","status":"healthy","version":"2.0"}
```

### Loading sample data

The database schema and seed data are automatically loaded on first startup via Docker entrypoint scripts. To load additional sample data:

```bash
docker exec -i library_postgres psql -U postgres -d librarydb < db/sample_data.sql
```

To run the digital media migration:

```bash
docker exec -i library_postgres psql -U postgres -d librarydb < db/migrations/001_digital_media.sql
```

### Building from source

```bash
cd Source/version-2.0
cmake -S . -B build
cmake --build build -j$(nproc)
./build/library_server
```

Requires all library dependencies to be installed on the system. See the `docker/Dockerfile` for the exact build steps and dependency versions.

### Running tests

```bash
cd docker
docker compose run --rm library_tests
```

Or locally:

```bash
cmake --build build --target run_tests
cd build && ctest --output-on-failure
```

## Configuration

All configuration is done through environment variables. Defaults are provided for Docker Compose deployments.

| Variable                 | Default                                              | Description                       |
|--------------------------|------------------------------------------------------|-----------------------------------|
| `POSTGRES_URI`           | `postgresql://postgres:password@postgres:5432/librarydb` | PostgreSQL connection string  |
| `MONGO_URI`              | `mongodb://root:password@mongo:27017`                | MongoDB connection string         |
| `MONGO_DB`               | `library_logs`                                       | MongoDB database name             |
| `MONGO_COLLECTION`       | `audit_events`                                       | MongoDB collection for logs       |
| `REST_HOST`              | `0.0.0.0`                                            | REST API bind address             |
| `REST_PORT`              | `8080`                                               | REST API port                     |
| `GRPC_HOST`              | `0.0.0.0`                                            | gRPC bind address                 |
| `GRPC_PORT`              | `50051`                                              | gRPC port                         |
| `JWT_SECRET`             | `super_secret_jwt_key`                               | JWT signing secret                |
| `JWT_EXPIRATION_MINUTES` | `6000`                                               | JWT token expiry in minutes       |
| `OPENSEARCH_URL`         | `http://opensearch:9200`                             | OpenSearch endpoint               |
| `REDIS_HOST`             | `redis`                                              | Redis hostname                    |
| `REDIS_PORT`             | `6379`                                               | Redis port                        |
| `REDIS_PASSWORD`         | (empty)                                              | Redis password                    |
| `S3_ENDPOINT`            | `http://minio:9000`                                  | MinIO/S3 endpoint                 |
| `S3_ACCESS_KEY`          | `minioadmin`                                         | S3 access key                     |
| `S3_SECRET_KEY`          | `minioadmin`                                         | S3 secret key                     |
| `S3_BUCKET`              | `library-media`                                      | S3 bucket for digital media       |
| `S3_REGION`              | `us-east-1`                                          | S3 region                         |
| `KAFKA_BROKERS`          | `kafka:9092`                                         | Kafka broker addresses            |
| `QUEUE_INTERVAL`         | `2`                                                  | Queue worker poll interval (sec)  |

## Running the Application

### Start all services

```bash
cd Source/version-2.0/docker
docker compose up -d
```

### Stop all services

```bash
docker compose down
```

### Stop and remove all data

```bash
docker compose down -v
```

### View logs

```bash
docker compose logs -f library_backend
```

## API Reference

### Authentication

All protected endpoints require a JWT token in the `Authorization` header:

```
Authorization: Bearer <token>
```

### Public Endpoints

| Method | Path             | Description            |
|--------|------------------|------------------------|
| POST   | `/api/login`     | Authenticate, get JWT  |
| POST   | `/api/register`  | Register a new user    |
| GET    | `/api/media`     | List all media         |
| GET    | `/health`        | Health check           |
| GET    | `/ready`         | Readiness check        |
| GET    | `/metrics`       | Prometheus metrics     |

### Protected Endpoints

| Method | Path                                    | Description                        | Roles           |
|--------|-----------------------------------------|------------------------------------|-----------------|
| GET    | `/api/users`                            | List all users                     | ADMIN           |
| POST   | `/api/users`                            | Create a user                      | ADMIN           |
| POST   | `/api/users/assign-role`                | Assign role to user                | ADMIN           |
| POST   | `/api/media/book`                       | Create a book                      | LIBRARIAN+      |
| POST   | `/api/borrow`                           | Borrow a media copy                | MEMBER+         |
| POST   | `/api/return`                           | Return a media copy                | MEMBER+         |
| GET    | `/api/search?q=<query>`                 | Full-text search (fuzzy)           | Any auth user   |
| GET    | `/api/search/suggest?q=<prefix>`        | Auto-suggest completions           | Any auth user   |
| POST   | `/api/digital-media/upload`             | Upload digital media               | LIBRARIAN+      |
| GET    | `/api/digital-media/<id>/download-url`  | Get presigned download URL         | Any auth user   |
| GET    | `/api/digital-media/<id>/upload-url`    | Get presigned upload URL           | LIBRARIAN+      |
| POST   | `/api/digital-media/<id>/version`       | Create new file version            | LIBRARIAN+      |
| GET    | `/api/digital-media/<id>/versions`      | List file versions                 | Any auth user   |
| GET    | `/api/digital-media/<id>`               | Get digital media metadata         | Any auth user   |
| DELETE | `/api/digital-media/<id>`               | Delete digital media               | ADMIN           |
| POST   | `/api/import/json`                      | Bulk import from JSON              | LIBRARIAN+      |
| POST   | `/api/import/csv`                       | Bulk import from CSV               | LIBRARIAN+      |

### Example: Login and borrow a book

```bash
# Login
TOKEN=$(curl -s -X POST http://localhost:8080/api/login \
  -H "Content-Type: application/json" \
  -d '{"email":"emma.s@school.edu","password":"password123"}' | jq -r '.token')

# Search for a book
curl -s http://localhost:8080/api/search?q=mockingbird \
  -H "Authorization: Bearer $TOKEN" | jq

# Borrow copy ID 1
curl -s -X POST http://localhost:8080/api/borrow \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"user_id":5,"copy_id":1}'

# Return it
curl -s -X POST http://localhost:8080/api/return \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"user_id":5,"copy_id":1}'
```

### Example: Bulk import from CSV

```bash
TOKEN=$(curl -s -X POST http://localhost:8080/api/login \
  -H "Content-Type: application/json" \
  -d '{"email":"carol.lib@library.edu","password":"password123"}' | jq -r '.token')

curl -s -X POST http://localhost:8080/api/import/csv \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: text/csv" \
  --data-binary @db/sample_data.csv
```

## Database Schema

### Core Tables

```
media_type          -- Book, Magazine, DVD, Audiobook, DigitalMedia
media               -- Base media record (title, type, availability)
  book              -- author, isbn
  magazine          -- issue_number, publisher
  dvd               -- director
  audiobook         -- narrator
  digital_media     -- mime_type, s3_key, file_size, drm, version
media_copy          -- Physical copies with condition tracking
```

### User Tables

```
users               -- Base user (name, email, password_hash)
  admins            -- Admin subtype
  librarians        -- Librarian subtype
  members           -- Member subtype (borrow_limit)
    students        -- grade_level
    teachers        -- department
```

### Access Control

```
roles               -- ADMIN, LIBRARIAN, MEMBER, TEACHER, STUDENT
permissions         -- action + table_name pairs
role_permissions    -- Maps roles to permissions
user_roles          -- Maps users to roles
```

### Operations

```
active_borrow       -- Current active borrows
borrow_history      -- Completed borrows
task_queue          -- Async task persistence (PostgreSQL-backed)
digital_media_version -- File version history with checksums
```

### Stored Procedures

- `create_media_copy(media_id, condition)` -- Add a physical copy
- `add_active_borrow(user_id, copy_id)` -- Create borrow and mark copy unavailable
- `mark_copy_returned(copy_id)` -- Mark copy available
- `sync_media_availability(media_id)` -- Sync parent media availability from copies

Triggers automatically sync `media.is_available` when copies change and notify on permission changes.

## Sample Data

The `db/seed.sql` file loads on first startup and includes:

- 4 media types
- 27 media items (12 books, 5 magazines, 6 DVDs, 4 audiobooks)
- 51 physical copies
- 10 users across 5 roles
- 5 active borrows and 14 historical records
- Role-based permissions

Default password for all seed users: `password123`

### Test accounts

| Name             | Email                    | Role      |
|------------------|--------------------------|-----------|
| Alice Admin      | alice.admin@library.edu  | ADMIN     |
| Carol Librarian  | carol.lib@library.edu    | LIBRARIAN |
| Emma Student     | emma.s@school.edu        | STUDENT   |
| Iris Teacher     | iris.t@school.edu        | TEACHER   |

For a larger dataset, run `db/sample_data.sql` (see [Sample Data](#sample-data) section above).

## Monitoring

### Prometheus

Accessible at `http://localhost:9090`. Scrapes the application `/metrics` endpoint every 15 seconds.

Available metrics:

- `http_requests_total` -- Total HTTP requests
- `http_request_duration_seconds` -- Request latency histogram
- `active_connections` -- Current active connections

### Grafana

Accessible at `http://localhost:3000`. Default login: `admin` / `admin`.

Add Prometheus as a data source with URL `http://prometheus:9090` to visualize metrics.

### Service UIs

| Service               | URL                        | Credentials        |
|-----------------------|----------------------------|---------------------|
| REST API              | http://localhost:8080       | JWT auth            |
| gRPC                  | localhost:50051             | --                  |
| Mongo Express         | http://localhost:8082       | admin / admin       |
| OpenSearch Dashboards | http://localhost:5601       | --                  |
| MinIO Console         | http://localhost:9001       | minioadmin / minioadmin |
| Prometheus            | http://localhost:9090       | --                  |
| Grafana               | http://localhost:3000       | admin / admin       |

## Project Structure

```
Source/version-2.0/
  main.cpp                              -- Application entry point
  CMakeLists.txt                        -- Build configuration
  .env                                  -- Environment variables
  db/
    schema.sql                          -- Database schema
    seed.sql                            -- Initial seed data
    sample_data.sql                     -- Extended sample dataset
    migrations/
      001_digital_media.sql             -- Digital media tables
  docker/
    Dockerfile                          -- Multi-stage build
    docker-compose.yml                  -- Service orchestration
    prometheus.yml                      -- Prometheus config
  proto/
    log_service.proto                   -- gRPC service definition
  src/
    api/
      controllers/                      -- REST route handlers
        LoginController
        UserController
        MediaController
        BorrowController
        ReturnController
        SearchController
        DigitalMediaController
        BatchImportController
        MetricsController
      middleware/
        JwtMiddleware                   -- JWT token validation
        PermissionMiddleware            -- Role-based access control
      grpc/
        LogServiceServer                -- gRPC log service
    application/
      services/
        AuthService                     -- Authentication
        UserService                     -- User management
        LibraryService                  -- Core borrow/return logic
        DigitalMediaService             -- Upload, download URLs, versioning
        BatchImportService              -- CSV/JSON bulk import
        PgQueueService                  -- PostgreSQL task queue
        PermissionService               -- Permission cache
    domain/
      media/
        Media, Book, Magazine, DVD, AudioBook, DigitalMedia, MediaCopy
      borrow/
        BorrowRecord, ActiveBorrow, BorrowHistory
      user/
        User, Admin, Librarian, Member, Student, Teacher, Role, Permission
    data/
      PostgresAdapter                   -- PostgreSQL queries
      MongoAdapter                      -- MongoDB audit log queries
    infrastructure/
      cache/
        RedisClient                     -- Redis get/set/sessions/invalidation
      config/
        ConfigManager                   -- Environment config loader
        EnvLoader                       -- .env file parser
      crypto/
        PasswordHasher                  -- bcrypt hashing
      db/
        PostgresPool                    -- Connection pooling
        MongoConnection                 -- MongoDB client
      jwt/
        JwtHelper                       -- JWT create/verify
      messaging/
        KafkaProducer                   -- Event publishing
        KafkaConsumer                   -- Event consumption
      metrics/
        MetricsRegistry                 -- Prometheus counters/gauges/histograms
      queue/
        PersistentQueue                 -- PostgreSQL-backed queue
        QueueWorker                     -- Background task processor
      search/
        OpenSearchClient                -- Full-text search, fuzzy, auto-suggest
      storage/
        S3StorageClient                 -- S3/MinIO upload, download, presigned URLs
    utils/
      Exceptions.h                      -- Exception hierarchy
      JsonUtils.h                       -- JSON parsing helpers
      StringUtils.h                     -- String utilities
      DateTimeUtils.h                   -- Timestamp formatting
  tests/
    test_auth_service.cpp               -- Auth integration tests
    test_user_service.cpp               -- User integration tests
```
