# Software Requirements Specification (SRS)

**Project Name:** Library Management System
**Version:** beta_0.2
**Date:** 04.11.2025
**Author:** FrozenPotato

---

## Introduction

### 1.1 Purpose  
This document defines the **functional and non-functional requirements** for the **Library Management System (LMS)** — a scalable system for managing both **digital** and **physical media**.  
It supports creation, retrieval, borrowing, and delivery of media files while ensuring high performance, security, and maintainability.

### 1.2 Scope  
The system enables:
- Management of over 1 million media records.  
- Handling of **digital media** (eBooks, PDFs, audio, video) via object storage.  
- Tracking and delivery of **physical media** through integrated logistics gateways.  
- Integration with authentication, logging, and analytics subsystems.  
- Scalability to 100,000 transactions per second (TPS).

### 1.3 Definitions and Acronyms

| Term | Definition |
|------|-------------|
| LMS | Library Management System |
| API | Application Programming Interface |
| JWT | JSON Web Token |
| CQRS | Command Query Responsibility Segregation |
| HA | High Availability |
| CDN | Content Delivery Network |
| RBAC | Role-Based Access Control |
| DDD | Domain-Driven Design |
| S3 / MinIO | Object Storage |
| Kafka / RabbitMQ | Message Brokers |

---

## Overall Description

### 2.1 Product Perspective  
LMS follows **Clean Architecture** principles with four layers:

```
API Layer → Application Layer → Domain Layer → Infrastructure Adapters
```

- **API Layer:** REST and gRPC interfaces for clients and external systems.  
- **Application Layer:** Implements business use cases (Library, Media, User).  
- **Domain Layer:** Core entities and rules independent of frameworks.  
- **Infrastructure:** Databases, queues, search engines, and storage services.

### 2.2 Product Functions  
- CRUD management of media metadata.  
- Upload and controlled access to digital media.  
- Borrow and return workflows with policy enforcement.  
- Physical delivery tracking via asynchronous queues.  
- Keyword and full-text search.  
- User authentication, authorization, and auditing.

### 2.3 User Classes and Roles

| Role | Description |
|------|-------------|
| **Administrator** | Manages system policies, permissions, imports. |
| **Librarian/Operator** | Handles inventory, borrow/return, delivery updates. |
| **End User** | Searches, borrows, and accesses media. |
| **Integrator** | Connects third-party delivery or analytics systems. |

### 2.4 Operating Environment  
- Deployed in **Docker/Kubernetes** clusters.  
- Load balancing via **Envoy/Nginx**.  
- Multi-AZ setup for high availability.  
- Clustered databases, caches, and queues.

---

## Functional Requirements

### 3.1 Media Lifecycle

| ID | Requirement | Description |
|----|--------------|-------------|
| FR-1 | Create Media | Create new media entries with metadata and file references. |
| FR-2 | Update Media | Edit metadata with version control. |
| FR-3 | Delete/Archive Media | Soft-delete or archive records with audit tracking. |
| FR-4 | Retrieve Media | Fetch metadata by ID, slug, or filters. |
| FR-5 | Bulk Import | Batch upload via CSV/JSON with validation. |

### 3.2 Digital Media Handling

| ID | Requirement | Description |
|----|--------------|-------------|
| FR-6 | Upload Media | Upload digital files. |
| FR-7 | Stream/Download | Access via signed URLs. |
| FR-8 | DRM / Access Limits | Optional per-user access restrictions. |
| FR-9 | File Versioning | Maintain version history for file updates. |

### 3.3 Physical Media Handling

| ID | Requirement | Description |
|----|--------------|-------------|
| FR-10 | Inventory Tracking | Each physical copy has unique status, location, condition. |
| FR-11 | Borrow Workflow | Borrow updates availability atomically. |
| FR-12 | Delivery Requests | Requests sent to Kafka for delivery processing. |
| FR-13 | Status Updates | Delivery Gateway updates returned to the system. |

### 3.4 Borrow / Return Management

| ID | Requirement | Description |
|----|--------------|-------------|
| FR-14 | Borrow Transaction | ACID transaction verifying availability. |
| FR-15 | Return Transaction | Return updates state and moves to history. |
| FR-16 | Overdue Detection | Scheduled job flags overdue items. |
| FR-17 | Policy Enforcement | Borrow durations/limits configurable by admin. |

### 3.5 Search & Discovery

| ID | Requirement | Description |
|----|--------------|-------------|
| FR-18 | Keyword Search | Title, author, tag-based queries. |
| FR-19 | Full-Text Search | Fuzzy match using OpenSearch. |
| FR-20 | Auto-Suggest | Predictive query completion. |
| FR-21 | Filter & Sort | Filter by type, tags, or availability. |

### 3.6 Eventing & Logging

| ID | Requirement | Description |
|----|--------------|-------------|
| FR-22 | Event Publishing | CRUD and workflow events to Kafka. |
| FR-23 | Audit Logging | Structured JSON logs. |
| FR-24 | Analytics Export | Send aggregated metrics to analytics service. |

### 3.7 Administration & RBAC

| ID | Requirement | Description |
|----|--------------|-------------|
| FR-25 | Role Management | RBAC enforcement with roles Admin/User/Librarian. |
| FR-26 | Permission Update | Immediate invalidation of cached access maps. |
| FR-27 | Audit Review | Admins view searchable audit logs. |

---

## Non-Functional Requirements

| Category | ID | Requirement | Target |
|-----------|----|--------------|--------|
| **Performance** | NFR-1 | Throughput ≥ 100,000 TPS | Load-tested |
| | NFR-2 | Read p95 < 120 ms, Write p95 < 250 ms | Metrics validated |
| **Scalability** | NFR-3 | Horizontal scaling for app and DB tiers | Linear to 3× load |
| **Availability** | NFR-4 | ≥ 99.95% uptime | Multi-AZ verified |
| **Security** | NFR-5 | TLS 1.3 + AES-256, JWT, Vault secrets | Full coverage |
| **Durability** | NFR-6 | No committed data loss | WAL replication |
| **Maintainability** | NFR-7 | Clean Architecture + 80% test coverage | CI pipelines |
| **Observability** | NFR-8 | Metrics, logs, traces for all components | OpenTelemetry |
| **Compliance** | NFR-9 | GDPR retention ≥ 3 years | Policy validation |
| **Recovery** | NFR-10 | RPO ≤ 5 min, RTO ≤ 30 min | DR drills |

---

## External Interface Requirements

| Interface | Protocol | Description |
|------------|-----------|-------------|
| **Media API** | REST / gRPC | CRUD, borrow, return operations |
| **Auth Service** | OAuth2 / JWT | Authentication and authorization |
| **Object Storage** | HTTP(S) / S3 API | Upload/download media |
| **Message Brokers** | Kafka / RabbitMQ | Event and background processing |
| **Search Engine** | REST / OpenSearch | Full-text indexing and queries |
| **Delivery Gateway** | REST / gRPC | Physical delivery integration |

---

## System Architecture Summary

### Logical Layers
1. **API Layer** – Crow REST & gRPC endpoints, JWT middleware.  
2. **Application Layer** – Use-case orchestration and business services.  
3. **Domain Layer** – Entities, value objects, policies.  
4. **Infrastructure Layer** – Adapters: DB, queues, search, storage.

### Physical Deployment
- **App Pods:** Stateless containers in Kubernetes.  
- **DB Layer:** PostgreSQL cluster.  
- **Cache:** Redis Cluster.  
- **Object Storage:** S3/MinIO replicated.  
- **Search:** OpenSearch cluster.  
- **Messaging:** Kafka (async) + RabbitMQ (interactive).  
- **Observability:** Prometheus + Grafana + Loki.

---

## Technology Justification

| Component | Technology | Justification |
|------------|-------------|---------------|
| **Language** | C++ 20 | High-performance runtime for 100k TPS target. |
| **Frameworks** | Crow (REST) + gRPC | Lightweight, async, and efficient. |
| **Architecture** | Clean Architecture + DDD | Maintainability and testability. |
| **DB (SQL)** | PostgreSQL | Strong consistency, partitioning, ACID. |
| **Cache** | Redis | Sub-ms latency, supports counters and pub/sub. |
| **Object Store** | S3 / MinIO | Scalable binary storage, CDN-ready. |
| **Search** | OpenSearch | Fast full-text and filter queries. |
| **Messaging** | RabbitMQ + Kafka | Low-latency user queues + high-throughput events. |
| **Logging** | MongoDB | Flexible schema for structured logs. |
| **Security** | OAuth2 + JWT | Industry-standard protection and secret management. |
| **Deployment** | Docker + Kubernetes | Auto-scaling and high availability. |
| **Monitoring** | Prometheus + Grafana + OpenTelemetry | Unified observability. |

---

## Assumptions and Dependencies
- Auth, delivery, and analytics systems available via API.  
- CDN configured for digital delivery.  
- Managed DB and MQ services recommended for production. 
---


