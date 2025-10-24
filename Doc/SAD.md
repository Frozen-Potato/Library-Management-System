# Software Architecture Document

**Project Name:** Library Management System
**Version:** beta_0.2
**Date:** 19.10.2025
**Author:** FrozenPotato

---

## Introduction

### Purpose

This document defines the architectural design of the Library System, focusing on its logical, physical, and deployment architecture.
It describes how REST, gRPC, and data storage components interact within a modular monolithic system.

### Goals

- Establish a maintainable and extensible architecture.
- Support large-scale datasets (≥ 1 million media items).
- Enable REST + gRPC services in one runtime process.
- Ensure security and auditability via JWT and MongoDB logs.

### Scope

The system manages library operations, including:
- Media catalog management
- Borrowing and returning media copies
- User registration and authentication
- Audit logging of system actions

It integrates PostgreSQL (for relational persistence) and MongoDB (for activity logs), both running in Docker.
REST APIs are implemented via Crow, and gRPC is used for internal log queries.

## System Overview

### Architecture Style

The Library System follows a Layered Modular Monolith architecture.
Although it runs as a single executable, it separates business logic from APIs and persistence layers.
The runtime hosts:
- A Crow REST server for public and protected HTTP endpoints
- A gRPC service for log queries
- Shared business logic and database adapters across both APIs

| Layer                    | Responsibility                                              |
| ------------------------ | ----------------------------------------------------------- |
| **Client Layer**         | External consumers (web / mobile / CLI / gRPC client).      |
| **API Layer**            | REST + gRPC endpoints, JWT middleware.                      |
| **Application Layer**    | Core business services (Library, Auth, User).               |
| **Domain Layer**         | Entities: Media, Borrow, User, Role, Permission.            |
| **Data Access Layer**    | `PostgresAdapter`, `MongoAdapter` for persistence.          |
| **Infrastructure Layer** | Configuration, DB connections, JWT helper, password hasher. |
| **Data Stores**          | PostgreSQL (main DB), MongoDB (audit logs).                 |

## Gereral Data flow

```
Clients  →  REST API / gRPC  →  Application Services  →  Adapters  →  Databases
```
- All client requests to protected pass through JWT middleware.
- REST and gRPC share the same service layer instance.
- Data persistence flows to PostgreSQL (core data) and MongoDB (audit events).

## Architecture Layers

### API Layer

Responsibilities:
- Expose REST and gRPC endpoints
- Validate incoming requests
- Handle authentication for protected routes
- Return structured JSON or protobuf responses

Key Components:
- `LoginController`, `MediaController`, `BorrowController`, `ReturnController`, `UserController`
- JwtMiddleware for Crow
- LogService for gRPC
Separation of routes:
- Public: `/media`, `/login`, `/register`
- Protected: `/borrow`, `/return`, `/users`, `/logs`

### Application Layer

Responsibilities:
- Implements business logic for borrowing, returning, and media searches
- Coordinates between domain and persistence layers
- Calls adapters for data access and Mongo logging

Key Components:
- `LibraryService` → media & borrow flow logic
- `AuthService` → authentication, registration, token issuance
- `UserService` → user management and role assignments

### Domain Layer
Responsibilities:
Define the business entities and rules that describe the library domain
Independent of database or frameworks

Main Entities:
- `Media` (abstract) → `Book`, `Magazine`, `DVD`, `AudioBook`
- `MediaCopy`
- `Borrow Models` → `ActiveBorrow`, `BorrowHistory`
- `User` hierarchy → `Member`, `Student`, `Teacher`, `Librarian`, `Admin`
- `Role` and `Permission` for **RBAC**

### Data Access Layer

Responsibilities:
- Abstract database interaction
- Isolate SQL/NoSQL code from the business layer

Components:
`PostgresAdapter`: handles relational **CRUD** for `media`, `copies`, `borrows`, `users`, and `roles`.
`MongoAdapter`: logs and fetches audit entries in **MongoDB**.
Both adapters receive configuration from `EnvLoader`.

### Infrastructure Layer

Responsibilities:
- Manage cross-cutting and external dependencies

Components:
- `JwtHelper` → token creation & validation
- `PasswordHasher` → secure credential storage
- `EnvLoader` / `ConfigManager` → load .env and runtime variables
- Database pools → `pqxx::connection_pool`, `mongocxx::client`

## Runtime View
### Startup Sequence

```
main.cpp
 ├─ Load configuration from .env
 ├─ Initialize Postgres & Mongo connections
 ├─ Initialize services (LibraryService, AuthService, UserService)
 ├─ Start Crow server (REST)
 │   ├─ Register public routes
 │   ├─ Attach JwtMiddleware for protected routes
 │   └─ Run on thread pool
 └─ Start gRPC server (LogService)
```
### Simplified borrowing flow

```
1. Client → POST /borrow (JWT)
2. JwtMiddleware validates token, extracts user_id
3. BorrowController → LibraryService.borrowCopy(user_id, copy_id)
4. LibraryService:
     - Validates copy availability
     - Inserts record via PostgresAdapter
     - Logs action via MongoAdapter
5. Response → 200 OK
```

## Deployment View

Runtime Processes:
Single binary: `library_server`
Thread 1–N: Crow REST API
Thread M: gRPC Log Service

Docker containers:
`postgres`
`mongo`
`library_server`

Network:
```
library_network
├── postgres (5432)
├── mongo (27017)
└── library_server (8080 REST, 50051 gRPC)
```
## Data View
The PostgreSQL schema follows a polymorphic relational model with subtype tables and role-based access.

| Table                                      | Description                                 |
| ------------------------------------------ | ------------------------------------------- |
| `MEDIA_TYPE`                               | Defines category (Book, DVD, etc.)          |
| `MEDIA`                                    | Abstract media entry linked to `MEDIA_TYPE` |
| `BOOK`, `MAGAZINE`, etc.                   | Subtype detail tables                       |
| `MEDIA_COPY`                               | Physical copies                             |
| `ACTIVE_BORROW`, `BORROW_HISTORY`          | Transaction tracking                        |
| `USERS`, `MEMBERS`, `ADMINS`, `LIBRARIANS` | User hierarchy                              |
| `ROLES`, `PERMISSIONS`                     | RBAC metadata                               |

MongoDB stores:
- Audit events (`BORROW`, `RETURN`, `LOGIN`)
- Structured JSON documents with timestamps and user context

## Security View

| Area                     | Mechanism                                      |
| ------------------------ | ---------------------------------------------- |
| **Authentication**       | JWT (HMAC SHA256) via `JwtHelper`              |
| **Authorization**        | Role-based checks (`Admin`, `Librarian`, etc.) |
| **Password Storage**     | Hashed via bcrypt/argon2                       |
| **Data Confidentiality** | TLS termination at reverse proxy (e.g., Nginx) |
| **Audit**                | All critical actions logged in MongoDB         |
