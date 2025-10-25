# Software Design Document

**Project Name:** Library Management System
**Version:** beta_0.2
**Date:** 19.10.2025
**Author:** FrozenPotato

---

## Introduction
### Purpose

This document details the design of classes, components, and their interactions in the Library System. It serves as a guide for implementation, maintenance, and future enhancements.

### Scope

The Library System is a C++20 modular monolith with REST (Crow) and gRPC interfaces. It integrates PostgreSQL for structured data and MongoDB for logging.

### Design Goals

- Maintainable and extensible class structure
- Clear separation between layers
- Efficient data access and caching
- Secure authentication (JWT)
- High performance for 1M+ media entries

## Class Diagram (UML)
```mermaid
classDiagram
    %% === Domain - Media ===
    class MediaType {
        +id : int
        +name : string
        +description : string
        +getId() int
        +getName() string
    }

    class Media {
        <<abstract>>
        #id : int
        #title : string
        #mediaTypeId : int
        #isAvailable : bool
        +getId() int
        +getTitle() string
        +getMediaTypeId() int
        +isAvailable() bool
        +setAvailability(bool)
    }

    class Book {
        -author : string
        -isbn : string
        +getAuthor() string
        +getIsbn() string
    }

    class Magazine {
        -issueNumber : int
        -publisher : string
        +getIssueNumber() int
        +getPublisher() string
    }

    class DVD {
        -durationMinutes : int
        -director : string
        +getDuration() int
        +getDirector() string
    }

    class AudioBook {
        -lengthMinutes : int
        -narrator : string
        +getLength() int
        +getNarrator() string
    }

    class MediaCopy {
        -copyId : int
        -mediaId : int
        -condition : string
        -isAvailable : bool
        +getCopyId() int
        +getMediaId() int
        +getCondition() string
        +isAvailable() bool
        +setAvailability(bool)
    }

    %% === Domain - Borrow ===
    class ActiveBorrow {
        -borrowId : int
        -userId : int
        -copyId : int
        -borrowDate : DateTime
        -dueDate : DateTime
        +getBorrowId() int
        +getUserId() int
        +getCopyId() int
        +isOverdue() bool
    }

    class BorrowHistory {
        -borrowId : int
        -userId : int
        -copyId : int
        -borrowDate : DateTime
        -returnDate : DateTime
        +getBorrowId() int
        +getDuration() int
    }

    %% === Domain - Users ===
    class User {
        <<abstract>>
        #id : int
        #name : string
        #email : string
        #passwordHash : string
        #userType : string
        #createdAt : DateTime
        +getId() int
        +getName() string
        +getEmail() string
        +getUserType() string
        +authenticate(password: string) bool
    }

    class Member {
        #role : string
        #borrowLimit : int
        +getRole() string
        +getBorrowLimit() int
        +canBorrow() bool
    }

    class Student {
        -gradeLevel : string
        +getGradeLevel() string
    }

    class Teacher {
        -department : string
        +getDepartment() string
    }

    class Librarian {
        -branch : string
        -shiftSchedule : string
        +getBranch() string
        +getShiftSchedule() string
    }

    class Admin {
        -accessLevel : string
        -department : string
        +getAccessLevel() string
        +getDepartment() string
    }

    %% === Domain - Roles ===
    class Role {
        -id : int
        -name : string
        -description : string
        +getId() int
        +getName() string
        +hasPermission(Permission) bool
    }

    class Permission {
        -id : int
        -name : string
        -description : string
        +getId() int
        +getName() string
    }

    %% === Services ===
    class LibraryService {
        -db : PostgresAdapter
        -logger : MongoAdapter
        +borrowCopy(userId:int, copyId:int) bool
        +returnCopy(userId:int, copyId:int) bool
        +listAvailableCopies(mediaId:int) vector~MediaCopy~
        +searchMedia(query:string) vector~Media~
    }

    class AuthService {
        -db : PostgresAdapter
        -jwt : JwtHelper
        -hasher : PasswordHasher
        +login(email:string, password:string) string
        +validateToken(token:string) User
        +register(User) int
    }

    class UserService {
        -db : PostgresAdapter
        +listUsers() vector~User~
        +assignRole(userId:int, roleId:int)
    }

    %% === Infrastructure ===
    class JwtHelper {
        -secretKey : string
        +createToken(userId:int, role:string) string
        +validateToken(token:string) bool
        +extractUserId(token:string) int
    }

    class PasswordHasher {
        +hashPassword(password:string) string
        +verify(password:string, hash:string) bool
    }

    class ConfigManager {
        +get(key:string) string
    }

    class PostgresAdapter {
        +addBorrowRecord(userId:int, copyId:int)
        +markReturned(borrowId:int)
        +getUserByEmail(email:string)
        +addUser(User)
        +getAllMedia()
    }

    class MongoAdapter {
        +logAction(action:string, userId:int, entityId:int)
        +fetchLogs(filter)
    }

    class Task {
        +id : int
        +type : string
        +payload : json
        +status : string
        +createdAt : time_point
        +Task(id:int, type:string, payload:json, status:string)
    }

    class PersistentQueue {
        -connection : PostgresConnection
        -mutex : std::mutex
        +enqueue(task: Task) void
        +dequeue() optional~Task~
        +markProcessing(id:int) void
        +markDone(id:int) void
        +markFailed(id:int, reason:string) void
        +countPending() int
    }

    class QueueWorker {
        -queue : PersistentQueue*
        -handlers : map~string, ITaskHandler*~
        -running : bool
        +start() void
        +stop() void
        -loop() void
        -process(task: Task) void
        +registerHandler(type:string, handler:ITaskHandler*) void
    }

    class ITaskHandler {
        <<interface>>
        +handle(task: Task) void
    }

    class LogEventHandler {
        -mongoLogger : MongoLogger
        +handle(task: Task) void
    }

    class SyncMediaHandler {
        -db : PostgresAdapter
        +handle(task: Task) void
    }

    %% === Relationships ===
    Media <|-- Book
    Media <|-- Magazine
    Media <|-- DVD
    Media <|-- AudioBook
    User <|-- Member
    Member <|-- Student
    Member <|-- Teacher
    User <|-- Librarian
    User <|-- Admin
    Role "1" --> "*" Permission
    User "*" --> "*" Role
    LibraryService --> PostgresAdapter
    LibraryService --> MongoAdapter
    AuthService --> PostgresAdapter
    AuthService --> JwtHelper
    AuthService --> PasswordHasher

    PersistentQueue "1" o-- "*" Task : manages
    QueueWorker --> PersistentQueue : uses
    QueueWorker --> ITaskHandler : delegates to
    ITaskHandler <|.. LogEventHandler
    ITaskHandler <|.. SyncMediaHandler
```

## Data Flow Summary

### Flows

| **Operation**          | **Execution Type**   | **Main Flow**                                                                                                         | **Async Tasks (via PersistentQueue)**                              |
| ---------------------- | -------------------- | --------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------ |
| **Borrow Copy**        | Synchronous          | REST → Controller → `LibraryService` → `PostgresAdapter` → PostgreSQL (insert `active_borrow`, mark copy unavailable) | Enqueue `LOG_EVENT` (Mongo), optionally enqueue `SYNC_MEDIA_STATE` |
| **Return Copy**        | Synchronous          | REST → Controller → `LibraryService` → `PostgresAdapter` → PostgreSQL (move to history, mark copy available)          | Enqueue `LOG_EVENT` (Mongo), optionally enqueue `SYNC_MEDIA_STATE` |
| **Login**              | Synchronous          | REST → `AuthService` → `PostgresAdapter` → `PasswordHasher` → `JwtHelper`                                             | None                                                               |
| **Search Media**       | Synchronous          | REST → `LibraryService` → `PostgresAdapter` (SQL `LIKE` query)                                                        | None                                                               |
| **gRPC Log Stream**    | Synchronous (stream) | gRPC → `MongoAdapter::fetchLogs()`                                                                                    | None — reads directly from Mongo                                   |
| **Background Logging** | Asynchronous         | `LibraryService` → `PersistentQueue` → `QueueWorker` → `MongoLogger`                                                  | Queue writes to Mongo, decoupled from REST latency                 |
| **Background Sync**    | Asynchronous         | `LibraryService` → `PersistentQueue` → `QueueWorker` → `PostgresAdapter::syncMediaAvailability()`                     | Keeps media availability consistent                                |


### Behavior Summary

| **Aspect**           | **Synchronous Path**                                              | **Asynchronous Path**                                            |
| -------------------- | ----------------------------------------------------------------- | ---------------------------------------------------------------- |
| **Goal**             | Ensure core business transaction (borrow, return, login) succeeds | Offload non-critical side effects (logging, sync, notifications) |
| **Components**       | REST/gRPC controllers, services, adapters                         | PersistentQueue, QueueWorker, task handlers                      |
| **Latency**          | Directly affects API response                                     | Runs in background thread/process                                |
| **Failure Handling** | Rolls back transaction                                            | Retries via queue, no user-facing error                          |
| **Persistence**      | PostgreSQL                                                        | PostgreSQL (task_queue) + MongoDB (logs)                         |


## Core Algorithms

### Borrow Copy Logic

```cpp
bool LibraryService::borrowCopy(int userId, int copyId) {
    auto copy = db.getCopy(copyId);
    if (!copy.isAvailable()) return false;

    db.addBorrowRecord(userId, copyId);
    db.updateCopyAvailability(copyId, false);
    logger.logAction("BORROW", userId, copyId);
    return true;
}
```

### Return Copy Logic

```cpp
bool LibraryService::returnCopy(int userId, int copyId) {
    auto record = db.findActiveBorrow(userId, copyId);
    if (!record) return false;

    db.markReturned(record.borrowId);
    db.updateCopyAvailability(copyId, true);
    logger.logAction("RETURN", userId, copyId);
    return true;
}
```

### Login & JWT Issue

```cpp
std::string AuthService::login(const std::string& email, const std::string& password) {
    auto user = db.getUserByEmail(email);
    if (!hasher.verify(password, user.passwordHash))
        throw std::runtime_error("Invalid credentials");

    return jwt.createToken(user.id, user.role);
}
```

### Mongo Logging

```cpp
void MongoAdapter::logAction(const std::string& action, int userId, int entityId) {
    nlohmann::json log = {
        {"action", action},
        {"user_id", userId},
        {"entity_id", entityId},
        {"timestamp", current_time_iso8601()}
    };
    mongoCollection.insert_one(bsoncxx::from_json(log.dump()));
}
```

### Database Interaction Example

```cpp
void PostgresAdapter::addBorrowRecord(int userId, int copyId) {
    pqxx::work txn(conn);
    txn.exec_params(
        "INSERT INTO active_borrow (user_id, copy_id, borrow_date) VALUES ($1, $2, NOW())",
        userId, copyId
    );
    txn.commit();
}
```

### Configuration

`.env` example:

```ini 
POSTGRES_URI=postgresql://user:password@localhost:5432/librarydb
MONGO_URI=mongodb://localhost:27017
JWT_SECRET=supersecretkey
```

## Threading & Concurrency
- Crow handles REST requests asynchronously.
- gRPC runs concurrently in a detached thread.
- PostgreSQL and Mongo clients each maintain separate connection pools.
- Mutex protection for shared logging operations.

## Error Handling
| Layer           | Handling Strategy                                       |
| --------------- | ------------------------------------------------------- |
| **REST**        | HTTP status codes + JSON message                        |
| **gRPC**        | `grpc::Status` objects                                  |
| **Services**    | C++ exceptions translated to user messages              |
| **DB Adapters** | Caught `pqxx::sql_error` or `mongocxx::query_exception` |
| **Global**      | Centralized error logger (Mongo)                        |


## Design Considerations

| Topic               | Design Decision                                         |
| ------------------- | ------------------------------------------------------- |
| **Extensibility**   | Adding new media type = add new subclass + DB table     |
| **Performance**     | SQL indexes on `media.title`, `copy_id`, `user_id`      |
| **Scalability**     | Future split: REST ↔ Auth ↔ Logging microservices       |
| **Security**        | JWT exp + HMAC SHA256, Argon2id for passwords           |
| **Maintainability** | Consistent interface names, one-responsibility services |
