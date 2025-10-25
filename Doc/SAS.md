# Software Architecture Specification

**Project Name:** Library Management System
**Version:** beta_0.2
**Date:** 19.10.2025
**Author:** FrozenPotato

---

## Overview

This document defines the Software Architecture Specification for the Library System, a layered modular monolith written in C++20, built with CMake, running a Crow-based REST API and a gRPC log service in the same process.

The system integrates:
- PostgreSQL (for relational persistence)
- MongoDB (for audit and log events)
- JWT-based authentication and authorization
- A lightweight Persistent Queue subsystem for asynchronous task handling

Both databases run in Docker containers. The REST and gRPC services share the same executable (library_server), with independent threads for each server.

## Architectural Context

### Layered Structure

The architecture is divided into five layers:
| Layer                    | Responsibility                                    | Example Components                                    |
| ------------------------ | ------------------------------------------------- | ----------------------------------------------------- |
| **API Layer**            | HTTP and gRPC endpoints, route handling, JWT auth | Crow Controllers, JwtMiddleware, gRPC Service         |
| **Application Layer**    | Core orchestration, business logic, use cases     | LibraryService, AuthService, UserService              |
| **Domain Layer**         | Core business entities, aggregates, and logic     | Media, Member, BorrowRecord, Role                     |
| **Data Access Layer**    | Database access and persistence abstraction       | PostgresAdapter, MongoAdapter                         |
| **Infrastructure Layer** | External systems integration, configuration       | JwtHelper, EnvLoader, PasswordHasher, PersistentQueue |


### Runtime Architecture

```mermaid
graph TD
    subgraph Clients
        Web[Web Client]
        Mobile[Mobile Client]
        GrpcCli[gRPC Client]
    end

    subgraph API Layer
        subgraph REST - Crow
            PublicGroup[Public Routes<br/>/media, /login, /register]
            ProtectedGroup[Protected Routes<br/>/borrow, /return, /users, /logs]
            JWT[JWT Middleware]
            LoginCtrl[Login Controller]
            MediaCtrl[Media Controller]
            BorrowCtrl[Borrow Controller]
            ReturnCtrl[Return Controller]
            UserCtrl[User Controller]
        end
        subgraph gRPC
            LogSvc[gRPC Log Service]
        end
    end

    subgraph Services
        LibSvc[LibraryService]
        AuthSvc[AuthService]
        UserSvc[UserService]
    end

    subgraph Domain
        MediaModels[Media Models<br/>Book, DVD, Magazine, AudioBook]
        UserModels[User Models<br/>Member, Student, Teacher, Librarian, Admin]
        BorrowModels[Borrow Models<br/>ActiveBorrow, BorrowHistory]
        RoleModels[Role & Permission Models]
    end

    subgraph Data Access
        PgAdapter[PostgresAdapter]
        MgAdapter[MongoAdapter]
    end

    subgraph Infrastructure
        JwtHelp[JwtHelper]
        Hasher[PasswordHasher]
        PgPool[PostgreSQL Connection Pool]
        MgConn[MongoDB Connection]
        Env[EnvLoader / ConfigManager]
        Queue[PersistentQueue]
        Worker[QueueWorker]
    end

    DB1[(PostgreSQL)]
    DB2[(MongoDB)]

    %% Connections
    Web --> PublicGroup
    Mobile --> PublicGroup
    GrpcCli --> LogSvc

    PublicGroup --> LoginCtrl
    PublicGroup --> MediaCtrl

    Web --> JWT
    Mobile --> JWT
    JWT --> ProtectedGroup
    ProtectedGroup --> BorrowCtrl
    ProtectedGroup --> ReturnCtrl
    ProtectedGroup --> UserCtrl

    LoginCtrl --> AuthSvc
    MediaCtrl --> LibSvc
    BorrowCtrl --> LibSvc
    ReturnCtrl --> LibSvc
    UserCtrl --> UserSvc
    LogSvc --> MgAdapter

    LibSvc --> PgAdapter
    LibSvc --> Queue
    Queue --> Worker
    Worker --> MgAdapter
    AuthSvc --> PgAdapter
    AuthSvc --> JwtHelp
    AuthSvc --> Hasher
    UserSvc --> PgAdapter

    PgAdapter --> PgPool --> DB1
    MgAdapter --> MgConn --> DB2
    JWT --> JwtHelp
    Env --> PgPool
    Env --> MgConn
    Env --> JwtHelp
    Env --> Queue

    class LoginCtrl,MediaCtrl,BorrowCtrl,ReturnCtrl,UserCtrl,JWT,PublicGroup,ProtectedGroup,LogSvc apiLayer
    class LibSvc,AuthSvc,UserSvc serviceLayer
    class MediaModels,UserModels,BorrowModels,RoleModels domainLayer
    class PgAdapter,MgAdapter dataLayer
    class JwtHelp,Hasher,PgPool,MgConn,Env,Queue,Worker infraLayer
    class DB1,DB2 dbLayer
```

## Component Specification

### API Layer

**Crow REST API**
- **Framework:** Crow
- **Endpoints:**
    - **Public (No JWT)**
    `/media` (GET), `/login` (POST), `/register` (optional)
    - **Protected (JWT Required)**
    `/borrow`, `/return`, `/users`, `/logs`
- Middleware: `JwtMiddleware` validates JWT and attaches user context.

**gRPC API**
- Service: `logservice.LogService`
- Purpose: Stream and query logs stored in MongoDB.
- Definition: `schema/log_service.proto`

### Middleware: JWT

Responsibilities:
1. Extracts `Authorization: Bearer <token>`.
2. Validates token signature and expiry.
3. Injects user context (`user_id`, `role`) into request.
4. Rejects unauthorized requests with `401 Unauthorized`.

Implementation Example:

```cpp
auto token = req.get_header_value("Authorization");
auto claims = jwtHelper.validateToken(token);
req.set_context<JwtContext>({claims.userId, claims.role});
```

### Application Services

| Service            | Key Responsibilities                                    |
| ------------------ | ------------------------------------------------------- |
| **LibraryService** | Borrow/return copies, manage availability, search media |
| **AuthService**    | Login, register, issue JWT tokens                       |
| **UserService**    | User CRUD and role management                           |

Each service:
- Calls domain model methods (validation, rules)
- Interacts with adapters for persistence
- Optionally logs via `MongoAdapter`

### Domain Models

| Entity                 | Description                                                       |
| ---------------------- | ----------------------------------------------------------------- |
| **Media / Subclasses** | Abstract representation of a book, magazine, DVD, etc.            |
| **MediaCopy**          | Physical copy of a media item; borrow/return works at this level  |
| **Borrow Models**      | `ActiveBorrow` and `BorrowHistory` track user transactions        |
| **User Hierarchy**     | `User` → `Member` → `Student` / `Teacher` / `Librarian` / `Admin` |
| **Roles/Permissions**  | Defines fine-grained access control for Admin functions           |

### Data Access Layer

- **PostgresAdapter**:
**CRUD** for `MEDIA`, `MEDIA_COPY`, `BORROW_HISTORY`, `USERS`, `ROLES`, etc.

- **MongoAdapter**:
Writes event logs (`BORROW`, `RETURN`, `LOGIN`, etc.) and queries via filters.

- **Connection**:
Uses `pqxx::connection` and `mongocxx::client` objects initialized from `.env`.

### Infrastructure Components

| Component           | Description                                              |
| ------------------- | -------------------------------------------------------- |
| **JwtHelper**       | Creates & validates JWTs using HMAC SHA256               |
| **PasswordHasher**  | Uses bcrypt/argon2 for secure password hashing           |
| **EnvLoader**       | Loads `.env` values for DB URIs and secrets              |
| **ConfigManager**   | Central access point for configuration values            |
| **PersistentQueue** | Internal durable queue storing async tasks in PostgreSQL |
| **QueueWorker**     | Background thread polling queue and executing operations |

The `PersistentQueue` subsystem ensures fault-tolerant asynchronous execution.
It stores tasks like log entries or delayed syncs in a PostgreSQL table (`task_queue`) and executes them via `QueueWorker`.
Tasks are retried automatically if a failure occurs, ensuring resilience and consistency without blocking user-facing operations.

### Data Model Overview

- Fully normalized schema with polymorphic media handling.
- New `MEDIA_TYPE` table decouples subtype definitions.
- `ACTIVE_BORROW` and `BORROW_HISTORY` separate for scalability.
- `TASK_QUEUE` table supports background task management.

```mermaid
erDiagram
    %% === Media Type Reference ===
    MEDIA_TYPE {
        int id PK
        text name UK
        text description
    }

    %% === Core Abstract Entity ===
    MEDIA {
        bigint id PK
        text title
        int media_type_id FK
        boolean is_available
    }

    %% === Subtype Tables ===
    BOOK {
        bigint media_id PK "FK"
        text author
        text isbn
    }

    MAGAZINE {
        bigint media_id PK "FK"
        int issue_number
        text publisher
    }

    DVD {
        bigint media_id PK "FK"
        int duration_minutes
        text director
    }

    AUDIOBOOK {
        bigint media_id PK "FK"
        int length_minutes
        text narrator
    }

    %% === Physical Copies ===
    MEDIA_COPY {
        bigint copy_id PK
        bigint media_id FK
        text condition
        boolean is_available
    }

    %% === Borrowing & Tracking - Active ===
    ACTIVE_BORROW {
        bigint borrow_id PK
        int user_id FK
        bigint copy_id FK
        timestamptz borrow_date
        timestamptz due_date
    }

    %% === Borrowing & Tracking - Historical ===
    BORROW_HISTORY {
        bigint borrow_id PK
        int user_id FK
        bigint copy_id FK
        timestamptz borrow_date
        timestamptz return_date
    }

    %% === User Hierarchy ===
    USERS {
        int id PK
        text name
        text email
        text password_hash
        text user_type
        timestamptz created_at
    }

    MEMBERS {
        int id PK "FK"
        text role
        int borrow_limit
    }

    STUDENTS {
        int id PK "FK"
        text grade_level
    }

    TEACHERS {
        int id PK "FK"
        text department
    }

    LIBRARIANS {
        int id PK "FK"
        text branch
        text shift_schedule
    }

    ADMINS {
        int id PK "FK"
        text access_level
        text department
    }

    %% === Permissions and Roles ===
    ROLES {
        int id PK
        text name UK
        text description
    }

    PERMISSIONS {
        int id PK
        text name UK
        text description
    }

    ROLE_PERMISSIONS {
        int role_id FK
        int permission_id FK
    }

    USER_ROLES {
        int user_id FK
        int role_id FK
    }

    TASK_QUEUE {
        bigint task_id PK
        text task_type
        jsonb payload
        text status
        timestamptz created_at
        timestamptz updated_at
    }

    %% === Relationships ===
    MEDIA_TYPE ||--o{ MEDIA : "categorizes"
    
    MEDIA ||--|| BOOK : "subtype_of"
    MEDIA ||--|| MAGAZINE : "subtype_of"
    MEDIA ||--|| DVD : "subtype_of"
    MEDIA ||--|| AUDIOBOOK : "subtype_of"

    MEDIA ||--o{ MEDIA_COPY : "has"
    MEDIA_COPY ||--o{ ACTIVE_BORROW : "referenced_in"
    MEDIA_COPY ||--o{ BORROW_HISTORY : "referenced_in"
    MEMBERS ||--o{ ACTIVE_BORROW : "creates"
    MEMBERS ||--o{ BORROW_HISTORY : "created"

    USERS ||--|| MEMBERS : "extends"
    USERS ||--|| LIBRARIANS : "extends"
    USERS ||--|| ADMINS : "extends"
    MEMBERS ||--|| STUDENTS : "subtype_of"
    MEMBERS ||--|| TEACHERS : "subtype_of"

    ROLES ||--o{ ROLE_PERMISSIONS : "maps"
    PERMISSIONS ||--o{ ROLE_PERMISSIONS : "defines"
    USERS ||--o{ USER_ROLES : "assigned"
    ROLES ||--o{ USER_ROLES : "grants"

    TASK_QUEUE ||--o{ MongoDB : "used for log storage"
```

### Route Security Model

| Endpoint     | HTTP | Auth     | Controller               | Notes                |
| ------------ | ---- | -------- | ------------------------ | -------------------- |
| `/media`     | GET  | Optional       | `MediaController`        | Public, read-only    |
| `/media/:id` | GET  | Optional        | `MediaController`        | Public               |
| `/login`     | POST | Optional        | `LoginController`        | Issues JWT           |
| `/register`  | POST | Optional | `LoginController`        | Public or restricted |
| `/borrow`    | POST | Madatory        | `BorrowController`       | Authenticated borrow |
| `/return`    | POST | Madatory        | `ReturnController`       | Authenticated return |
| `/users`     | GET  | Madatory        | `UserController`         | Admin/Librarian only |
| `/logs`      | GET  | Madatory        | `UserController` or gRPC | Internal use         |


### Deployment View

- Containerized via `docker-compose.yml`:
    - `library_server` (Crow + gRPC)
    - `postgres`
    - `mongo`
- Shared Docker network for internal access.
- Queue worker runs inside the same `library_server` process, ensuring all operations share one transactional context.
- Environment variables injected at runtime:

```ini
POSTGRES_URI=postgresql://user:pass@postgres:5432/librarydb
MONGO_URI=mongodb://mongo:27017/logs
JWT_SECRET=<secret>
```

