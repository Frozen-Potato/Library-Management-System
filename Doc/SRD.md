# Software Requirements Document

**Project Name:** Library Management System
**Version:** beta_0.2
**Date:** 19.10.2025
**Author:** FrozenPotato

---

## Introduction

### Purpose

This document defines all functional and non-functional requirements for the *Library Management System* (LMS), a unified platform for managing library resources, users, and borrowing operations.

### Scope

The system provides both REST (Crow) and gRPC interfaces for internal and external clients to:
- Manage media (books, magazines, DVDs, audiobooks)
- Handle user types (members, students, teachers, librarians, admins)
- Perform borrowing and return transactions
- Record activity logs in MongoDB
- Authenticate and authorize users via JWT

### System Context
- **REST API:** Crow-based HTTP server
- **gRPC Server:** Concurrent thread in same binary
- **Databases:** PostgreSQL (relational data), MongoDB (audit logs)
- **Deployment:** Docker-compose with PostgreSQL and MongoDB containers

## Functional Requirements

| ID    | Category           | Requirement Description                                                            |
| ----- | ------------------ | ---------------------------------------------------------------------------------- |
| LMS-1  | Media Management   | Create, update, delete, and query media entities (Book, Magazine, DVD, AudioBook). |
| LMS-2  | Media Copies       | Handle physical copies for each media item, track availability and condition.      |
| LMS-3  | Borrow Operations  | Borrow and return media copies by member ID; update availability status.           |
| LMS-4  | Borrow History     | Maintain active borrows and historical records.                                    |
| LMS-5  | User Management    | Manage users (Students, Teachers, Librarians, Admins).                             |
| LMS-6  | Authentication     | Issue JWT tokens upon successful login (email + password).                         |
| LMS-7  | Authorization      | Validate JWT for protected API endpoints.                                          |
| LMS-8  | Logging            | Record all actions (borrow, return, login, error) in MongoDB audit store.          |
| LMS-9  | Search & Filtering | Provide search for title, author, or media type via REST queries.                  |
| LMS-10 | System Monitoring  | Expose basic health and status endpoints.                                          |

## Non-Functional Requirements

| Type            | Requirement                                                 |
| --------------- | ----------------------------------------------------------- |
| Performance     | Handle ≥ 1 million media records in PostgreSQL.             |
| Availability    | ≥ 99.999 % uptime target.                                     |
| Security        | JWT-based authentication; hashed passwords (Argon2/Bcrypt). |
| Scalability     | Support horizontal scaling via Docker replicas.             |
| Maintainability | Layered architecture with separated concerns.               |
| Portability     | Linux-based Docker containers.                              |
| Auditability    | All user actions logged in MongoDB with timestamps.         |
| Testability     | REST/gRPC endpoints covered by integration tests.           |

## External Interfaces

### REST API

| Endpoint  | Method | Description                       |
| --------- | ------ | --------------------------------- |
| `/login`  | POST   | Authenticate and return JWT token |
| `/medias` | GET    | List available media              |
| `/borrow` | POST   | Borrow a copy                     |
| `/return` | POST   | Return a copy                     |
| `/users`  | GET    | List users (admin only)           |

### gRPC API

- `LogService.GetLogs()` – stream audit logs from MongoDB.

### Assumptions and Dependencies

- Crow HTTP library v0.3 or later.
- gRPC C++ v1.6x+.
- PostgreSQL ≥ 15 and libpqxx ≥ 7.8.
- MongoDB ≥ 7.0 and mongocxx ≥ 3.8.
- Docker environment with `.env` configuration file.

### Constraints

- All data must persist to PostgreSQL and MongoDB within ACID semantics.
- No plain-text password storage.
- Clients  →  REST API / gRPC  →  Application Services  →  Adapters  →  Databases
Library operations must complete within ≤ 1 s on average for normal load.