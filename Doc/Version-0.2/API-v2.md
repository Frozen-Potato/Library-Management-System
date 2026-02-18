# Library Management System — REST API v2 Documentation

> **Base URL:** `http://<host>:8080`
> **Version:** 2.0
> **Framework:** Crow (C++20)
> **Authentication:** JWT Bearer token — include in every protected request as `Authorization: Bearer <token>`

---

## Table of Contents

1. [Common Conventions](#1-common-conventions)
2. [Authentication](#2-authentication)
3. [Users](#3-users)
4. [Media](#4-media)
5. [Borrow & Return](#5-borrow--return)
6. [Search](#6-search)
7. [Digital Media](#7-digital-media)
8. [Batch Import](#8-batch-import)
9. [System / Observability](#9-system--observability)
10. [Data Types Reference](#10-data-types-reference)
11. [Error Responses](#11-error-responses)
12. [Role & Permission Matrix](#12-role--permission-matrix)

---

## 1. Common Conventions

### Content Type

All request bodies and responses use `application/json` unless otherwise noted (CSV import uses `text/plain`).

### HTTP Status Codes

| Code | Meaning |
|------|---------|
| `200` | OK — success |
| `201` | Created — resource created |
| `207` | Multi-Status — partial success (batch operations) |
| `400` | Bad Request — validation error |
| `401` | Unauthorized — missing or invalid JWT |
| `404` | Not Found — resource does not exist |
| `500` | Internal Server Error |

### Standard Error Response

```jsonc
// All error responses follow this shape:
{
  "error": "string"   // human-readable error message
}
```

### Standard Success Response (simple operations)

```jsonc
{
  "message": "string"   // human-readable confirmation
}
```

---

## 2. Authentication

### POST `/api/login`

Authenticate a user and receive a JWT token.

**Auth required:** No

#### Request Body

```jsonc
{
  "email":    "string",   // required — user email address
  "password": "string"    // required — plaintext password
}
```

#### TypeScript Interface

```ts
interface LoginRequest {
  email:    string;
  password: string;
}
```

#### Response `200 OK`

```jsonc
{
  "token": "string"   // JWT token — use in Authorization header for all protected endpoints
}
```

#### TypeScript Interface

```ts
interface LoginResponse {
  token: string;
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `400` | Missing `email` or `password` |
| `401` | Invalid credentials |

#### Example

```bash
curl -X POST http://localhost:8080/api/login \
  -H "Content-Type: application/json" \
  -d '{"email": "alice.admin@library.edu", "password": "password123"}'
```

```json
{ "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..." }
```

---

## 3. Users

### POST `/api/register`

Self-register a new user account.

**Auth required:** No

#### Request Body

```jsonc
{
  "name":        "string",            // required
  "email":       "string",            // required — must be unique
  "password":    "string",            // required
  "role":        "string",            // required — see Role Values below
  "grade_level": "string | null",     // optional — required when role is STUDENT
  "department":  "string | null"      // optional — required when role is TEACHER
}
```

#### TypeScript Interface

```ts
type UserRole = "ADMIN" | "LIBRARIAN" | "MEMBER" | "STUDENT" | "TEACHER";

interface RegisterRequest {
  name:         string;
  email:        string;
  password:     string;
  role:         UserRole;
  grade_level?: string | null;   // required if role === "STUDENT"
  department?:  string | null;   // required if role === "TEACHER"
}
```

#### Response `201 Created`

```jsonc
{
  "message": "User registered successfully",
  "user_id": 42,          // integer — newly assigned user ID
  "role":    "STUDENT"    // string — role assigned (normalized to uppercase)
}
```

#### TypeScript Interface

```ts
interface RegisterResponse {
  message: string;
  user_id: number;
  role:    string;
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `400` | Missing required fields |
| `500` | Database error (e.g., duplicate email) |

---

### GET `/api/users`

List all registered users.

**Auth required:** Yes — any authenticated user (JWT must be valid)

#### Request

No body. No query parameters.

```bash
curl http://localhost:8080/api/users \
  -H "Authorization: Bearer <token>"
```

#### Response `200 OK`

Returns a JSON **array** of user objects. Shape varies by role:

```jsonc
// Base user (all roles)
[
  {
    "id":    1,               // integer
    "name":  "Alice Admin",   // string
    "email": "alice@lib.edu", // string
    "role":  "Admin"          // string
  },
  // Student — includes grade_level
  {
    "id":          5,
    "name":        "Emma Student",
    "email":       "emma@school.edu",
    "role":        "Student",
    "grade_level": "Grade 10"   // string — only present for students
  },
  // Teacher — includes department
  {
    "id":         7,
    "name":       "Iris Teacher",
    "email":      "iris@school.edu",
    "role":       "Teacher",
    "department": "Mathematics"   // string — only present for teachers
  }
]
```

#### TypeScript Interface

```ts
interface BaseUser {
  id:    number;
  name:  string;
  email: string;
  role:  string;
}

interface StudentUser extends BaseUser {
  grade_level: string;
}

interface TeacherUser extends BaseUser {
  department: string;
}

type User = BaseUser | StudentUser | TeacherUser;

type GetUsersResponse = User[];
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `401` | Missing or invalid JWT |
| `500` | Database error |

---

### POST `/api/users`

Create a user (admin action).

**Auth required:** Yes — JWT must be valid (ADMIN permission enforced by middleware)

#### Request Body

```jsonc
{
  "name":        "string",   // required
  "email":       "string",   // required
  "password":    "string",   // required
  "role":        "string",   // optional — defaults to "Member"
  "grade_level": "string",   // optional — for Student accounts
  "department":  "string"    // optional — for Teacher accounts
}
```

#### TypeScript Interface

```ts
interface CreateUserRequest {
  name:         string;
  email:        string;
  password:     string;
  role?:        UserRole;     // defaults to "Member"
  grade_level?: string;
  department?:  string;
}
```

#### Response `201 Created`

```jsonc
{
  "message": "User created with ID 12"   // string — includes the new user ID
}
```

#### TypeScript Interface

```ts
interface CreateUserResponse {
  message: string;
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `400` | Missing `name`, `email`, or `password` |
| `401` | Missing or invalid JWT |
| `500` | Database error |

---

### POST `/api/users/assign-role`

Assign a new role to an existing user.

**Auth required:** Yes — ADMIN only

#### Request Body

```jsonc
{
  "user_id": 5,          // integer — required
  "role":    "LIBRARIAN" // string — required — see Role Values
}
```

#### TypeScript Interface

```ts
interface AssignRoleRequest {
  user_id: number;
  role:    UserRole;
}
```

#### Response `200 OK`

```jsonc
{
  "message": "Role assigned successfully"
}
```

#### TypeScript Interface

```ts
interface AssignRoleResponse {
  message: string;
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `400` | Missing `user_id` or `role` |
| `401` | Missing/invalid JWT or caller is not ADMIN |
| `404` | User not found |
| `500` | Database error |

---

## 4. Media

### GET `/api/media`

Retrieve a list of all media items.

**Auth required:** No (public endpoint)

#### Request

No body. No query parameters.

#### Response `200 OK`

```jsonc
[
  {
    "id":    1,              // integer (long) — media ID
    "title": "Clean Code"   // string — media title
  },
  {
    "id":    2,
    "title": "National Geographic - April 2024"
  }
]
```

#### TypeScript Interface

```ts
interface MediaItem {
  id:    number;
  title: string;
}

type GetMediaResponse = MediaItem[];
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `500` | Database error |

---

### POST `/api/media/book`

Create a new book in the catalog.

**Auth required:** Yes — LIBRARIAN or ADMIN

#### Request Body

```jsonc
{
  "title":         "string",   // required
  "author":        "string",   // required
  "isbn":          "string",   // optional — defaults to ""
  "media_type_id": 1           // integer — optional — defaults to 1 (Book)
}
```

#### TypeScript Interface

```ts
interface CreateBookRequest {
  title:          string;
  author:         string;
  isbn?:          string;   // defaults to ""
  media_type_id?: number;   // defaults to 1
}
```

#### Response `201 Created`

```jsonc
{
  "message": "Book created with ID 28"   // string — includes the new media ID
}
```

#### TypeScript Interface

```ts
interface CreateBookResponse {
  message: string;
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `400` | Missing `title` or `author` |
| `500` | Database error |

---

## 5. Borrow & Return

### POST `/api/borrow`

Borrow a physical copy of a media item. The user ID is taken from the JWT token.

**Auth required:** Yes — any authenticated user

#### Request Body

```jsonc
{
  "copy_id": 7   // integer (long) — required — the physical copy to borrow
}
```

#### TypeScript Interface

```ts
interface BorrowRequest {
  copy_id: number;
}
```

#### Response `200 OK`

```jsonc
{
  "message": "Borrow successful"
}
```

#### TypeScript Interface

```ts
interface BorrowResponse {
  message: string;
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `400` | Missing `copy_id` |
| `401` | Missing or invalid JWT |
| `404` | Copy not found |
| `500` | Database error (e.g., copy already borrowed) |

---

### POST `/api/return`

Return a previously borrowed copy. The user ID is taken from the JWT token.

**Auth required:** Yes — any authenticated user

#### Request Body

```jsonc
{
  "copy_id": 7   // integer (long) — required — the copy to return
}
```

#### TypeScript Interface

```ts
interface ReturnRequest {
  copy_id: number;
}
```

#### Response `200 OK`

```jsonc
{
  "message": "Return successful"
}
```

#### TypeScript Interface

```ts
interface ReturnResponse {
  message: string;
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `400` | Missing `copy_id` |
| `401` | Missing or invalid JWT |
| `404` | Copy or active borrow not found |
| `500` | Database error |

---

## 6. Search

### GET `/api/search`

Full-text fuzzy search over media items (powered by OpenSearch when available; falls back to in-memory filter).

**Auth required:** Yes — any authenticated user

#### Query Parameters

| Parameter | Type | Required | Default | Description |
|-----------|------|----------|---------|-------------|
| `q` | `string` | Yes | — | Search query string |
| `from` | `integer` | No | `0` | Pagination offset (number of results to skip) |
| `size` | `integer` | No | `10` | Page size — number of results to return |
| `fuzziness` | `string` | No | `"AUTO"` | OpenSearch fuzziness: `"AUTO"`, `"0"`, `"1"`, `"2"` |

#### Response `200 OK`

```jsonc
{
  "total":   3,       // integer — number of results returned in this page
  "from":    0,       // integer — pagination offset echoed back
  "size":    10,      // integer — page size echoed back
  "results": [        // array — matching media objects (shape depends on OpenSearch index mapping)
    {
      "id":    1,
      "title": "Clean Code",
      "type":  "Book"
    }
  ]
}
```

#### TypeScript Interface

```ts
interface SearchResult {
  id:    number;
  title: string;
  type?: string;
  [key: string]: unknown;   // additional fields from OpenSearch _source
}

interface SearchResponse {
  total:   number;
  from:    number;
  size:    number;
  results: SearchResult[];
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `400` | Missing `q` parameter |
| `500` | Search engine error |

#### Example

```bash
curl "http://localhost:8080/api/search?q=clean+code&size=5&fuzziness=1" \
  -H "Authorization: Bearer <token>"
```

---

### GET `/api/search/suggest`

Auto-complete suggestions based on a prefix query.

**Auth required:** No header check in this route (OpenSearch suggest call only)

#### Query Parameters

| Parameter | Type | Required | Default | Description |
|-----------|------|----------|---------|-------------|
| `q` | `string` | Yes | — | Prefix string for auto-complete |
| `limit` | `integer` | No | `5` | Max number of suggestions to return |

#### Response `200 OK`

```jsonc
{
  "query":       "cle",                       // string — prefix echoed back
  "suggestions": ["Clean Code", "Cleopatra"]  // string[] — suggested completions
}
```

#### TypeScript Interface

```ts
interface SuggestResponse {
  query:       string;
  suggestions: string[];
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `400` | Missing `q` parameter |
| `500` | Suggest engine error |

---

## 7. Digital Media

Digital media endpoints manage files stored in S3/MinIO object storage. Files are identified by a `media_id` (integer).

### POST `/api/digital-media/upload`

Upload a new digital media file. The file content is sent as a base64-encoded string inside the JSON body.

**Auth required:** Yes — LIBRARIAN or ADMIN

#### Request Body

```jsonc
{
  "title":         "string",   // required — display title for the media
  "mime_type":     "string",   // required — MIME type (see Supported MIME Types below)
  "file_data":     "string",   // required — file content (base64-encoded string)
  "drm_protected": false       // boolean — optional — default: false
}
```

#### Supported MIME Types

| MIME Type | Extension |
|-----------|-----------|
| `application/pdf` | `.pdf` |
| `application/epub+zip` | `.epub` |
| `video/mp4` | `.mp4` |
| `audio/mpeg` | `.mp3` |
| `image/jpeg` | `.jpg` |
| `image/png` | `.png` |
| `application/octet-stream` | `.bin` (default) |

#### TypeScript Interface

```ts
interface UploadDigitalMediaRequest {
  title:          string;
  mime_type:      string;
  file_data:      string;    // base64-encoded file content
  drm_protected?: boolean;   // default: false
}
```

#### Response `201 Created`

```jsonc
{
  "media_id":        28,                         // integer (long) — newly created media ID
  "title":           "Annual Report 2024",       // string
  "mime_type":       "application/pdf",          // string
  "s3_key":          "digital-media/28/v1.pdf",  // string — internal S3 object key
  "file_size":       204800,                     // integer — size in bytes
  "drm_protected":   false,                      // boolean
  "current_version": 1,                          // integer — starts at 1
  "created_at":      "2026-02-18T10:00:00Z"      // string — ISO 8601 timestamp
}
```

#### TypeScript Interface

```ts
interface UploadDigitalMediaResponse {
  media_id:        number;
  title:           string;
  mime_type:       string;
  s3_key:          string;
  file_size:       number;
  drm_protected:   boolean;
  current_version: number;
  created_at:      string;   // ISO 8601
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `400` | Missing `title`, `mime_type`, or `file_data` |
| `401` | Missing or invalid JWT |
| `500` | Storage or database error |

---

### GET `/api/digital-media/{id}`

Get metadata for a digital media item.

**Auth required:** Yes — any authenticated user

#### Path Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `id` | `integer` | Digital media ID |

#### Response `200 OK`

```jsonc
{
  "media_id":        28,
  "title":           "Annual Report 2024",
  "mime_type":       "application/pdf",
  "s3_key":          "digital-media/28/v1.pdf",
  "file_size":       204800,
  "drm_protected":   false,
  "current_version": 1,
  "created_at":      "2026-02-18T10:00:00Z"
}
```

#### TypeScript Interface

```ts
interface DigitalMediaMetadata {
  media_id:        number;
  title:           string;
  mime_type:       string;
  s3_key:          string;
  file_size:       number;   // bytes
  drm_protected:   boolean;
  current_version: number;
  created_at:      string;   // ISO 8601
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `401` | Missing or invalid JWT |
| `404` | Media not found |
| `500` | Internal error |

---

### GET `/api/digital-media/{id}/download-url`

Generate a presigned S3 download URL for direct client download.

**Auth required:** Yes — any authenticated user

#### Path Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `id` | `integer` | Digital media ID |

#### Query Parameters

| Parameter | Type | Required | Default | Description |
|-----------|------|----------|---------|-------------|
| `expiry` | `integer` | No | `3600` | URL expiry duration in seconds |

#### Response `200 OK`

```jsonc
{
  "download_url": "https://minio:9000/library/digital-media/28/v1.pdf?X-Amz-Signature=...",
  "expires_in":   3600   // integer — seconds until the URL expires
}
```

#### TypeScript Interface

```ts
interface DownloadUrlResponse {
  download_url: string;   // presigned S3/MinIO URL — redirect the browser here to download
  expires_in:   number;   // seconds
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `400` | Invalid media ID |
| `401` | Missing or invalid JWT |
| `404` | Media not found in cache/storage |
| `500` | Storage error |

---

### GET `/api/digital-media/{id}/upload-url`

Generate a presigned S3 upload URL so the client can upload a file directly to object storage without routing the binary through the API server.

**Auth required:** Yes — LIBRARIAN or ADMIN

#### Path Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `id` | `integer` | Digital media ID |

#### Query Parameters

| Parameter | Type | Required | Default | Description |
|-----------|------|----------|---------|-------------|
| `mime_type` | `string` | No | `application/octet-stream` | MIME type for the uploaded file |

#### Response `200 OK`

```jsonc
{
  "upload_url": "https://minio:9000/library/digital-media/28/v2.pdf?X-Amz-Signature=...",
  "media_id":   28   // integer — the media ID for reference
}
```

#### TypeScript Interface

```ts
interface UploadUrlResponse {
  upload_url: string;   // presigned S3/MinIO URL — PUT the file content directly here
  media_id:   number;
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `401` | Missing or invalid JWT |
| `404` | Media not found |
| `500` | Storage error |

---

### POST `/api/digital-media/{id}/version`

Create a new version of an existing digital media item by uploading new file content.

**Auth required:** Yes — LIBRARIAN or ADMIN

#### Path Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `id` | `integer` | Digital media ID |

#### Request Body

```jsonc
{
  "file_data": "string"   // required — new file content (base64-encoded)
}
```

#### TypeScript Interface

```ts
interface CreateVersionRequest {
  file_data: string;   // base64-encoded file content
}
```

#### Response `201 Created`

```jsonc
{
  "media_id":  28,                        // integer (long)
  "version":   2,                         // integer — new version number
  "s3_key":    "digital-media/28/v2.pdf", // string — new S3 object key
  "file_size": 215040                     // integer — size in bytes
}
```

#### TypeScript Interface

```ts
interface CreateVersionResponse {
  media_id:  number;
  version:   number;
  s3_key:    string;
  file_size: number;   // bytes
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `400` | Missing `file_data` or invalid media ID |
| `401` | Missing or invalid JWT |
| `404` | Media not found |
| `500` | Storage error |

---

### GET `/api/digital-media/{id}/versions`

List all uploaded versions for a digital media item.

**Auth required:** Yes — any authenticated user

#### Path Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `id` | `integer` | Digital media ID |

#### Response `200 OK`

```jsonc
{
  "media_id": 28,
  "versions": [
    {
      "version_number": 1,
      "s3_key":         "digital-media/28/v1.pdf",
      "file_size":      204800,
      "checksum":       "sha256:abc123...",
      "uploaded_at":    "2026-02-18T10:00:00Z",
      "is_current":     false
    },
    {
      "version_number": 2,
      "s3_key":         "digital-media/28/v2.pdf",
      "file_size":      215040,
      "checksum":       "sha256:def456...",
      "uploaded_at":    "2026-02-18T12:30:00Z",
      "is_current":     true
    }
  ]
}
```

#### TypeScript Interface

```ts
interface FileVersion {
  version_number: number;
  s3_key:         string;
  file_size:      number;      // bytes
  checksum:       string;
  uploaded_at:    string;      // ISO 8601
  is_current:     boolean;
}

interface ListVersionsResponse {
  media_id: number;
  versions: FileVersion[];
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `401` | Missing or invalid JWT |
| `500` | Storage error |

---

### DELETE `/api/digital-media/{id}`

Delete a digital media item and its associated file from object storage.

**Auth required:** Yes — ADMIN only

#### Path Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `id` | `integer` | Digital media ID |

#### Response `200 OK`

```jsonc
{
  "message": "Digital media deleted"
}
```

#### TypeScript Interface

```ts
interface DeleteDigitalMediaResponse {
  message: string;
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `401` | Missing or invalid JWT |
| `500` | Storage or cache error |

---

## 8. Batch Import

Both endpoints accept bulk media records and return a summary. They use HTTP `207 Multi-Status` when some records fail and `200` when all succeed.

### POST `/api/import/json`

Bulk import media records from a JSON array.

**Auth required:** Yes — LIBRARIAN or ADMIN

#### Request Body

Accepts either a raw JSON **array** or a wrapped object:

```jsonc
// Option A — raw array
[
  { "title": "Book A", "author": "Author 1", "type": "book" },
  { "title": "Book B", "author": "Author 2", "type": "book" }
]

// Option B — wrapped object
{
  "records": [
    { "title": "Book A", "author": "Author 1", "type": "book" },
    { "title": "Book B", "author": "Author 2", "type": "book" }
  ]
}
```

#### TypeScript Interface

```ts
interface ImportRecord {
  title:   string;
  type?:   string;   // "book" | "magazine" | "dvd" | "audiobook"
  author?: string;   // for books
  isbn?:   string;   // for books
  [key: string]: unknown;   // additional fields passed through
}

type ImportJsonRequest = ImportRecord[] | { records: ImportRecord[] };
```

#### Response `200 OK` (all succeed) or `207 Multi-Status` (partial failure)

```jsonc
{
  "status":   "success",   // "success" | "partial"
  "total":    10,          // integer — total records submitted
  "imported": 9,           // integer — successfully imported
  "failed":   1,           // integer — records that failed
  "errors":   [            // string[] — one message per failed record
    "Record 3: duplicate ISBN 978-0-13-468599-1"
  ]
}
```

#### TypeScript Interface

```ts
interface ImportResponse {
  status:   "success" | "partial";
  total:    number;
  imported: number;
  failed:   number;
  errors:   string[];
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `400` | Invalid JSON or missing `records` array |
| `401` | Missing or invalid JWT |
| `500` | Internal error |

---

### POST `/api/import/csv`

Bulk import media records from a CSV payload.

**Auth required:** Yes — LIBRARIAN or ADMIN

#### Request Body

Send raw CSV as the request body. The first row must be the header row.

```
Content-Type: text/plain

title,author,isbn,type
Clean Code,Robert C. Martin,978-0-13-235088-4,book
The Pragmatic Programmer,David Thomas,978-0-13-595705-9,book
```

#### TypeScript Interface

```ts
// Send as plain text string — no JSON encoding
type ImportCsvRequest = string;   // raw CSV content
```

#### Response `200 OK` or `207 Multi-Status`

Same shape as the JSON import response:

```jsonc
{
  "status":   "success",
  "total":    2,
  "imported": 2,
  "failed":   0,
  "errors":   []
}
```

#### TypeScript Interface

```ts
// Same as ImportResponse above
interface ImportResponse {
  status:   "success" | "partial";
  total:    number;
  imported: number;
  failed:   number;
  errors:   string[];
}
```

#### Error Responses

| Status | Condition |
|--------|-----------|
| `400` | Empty body |
| `401` | Missing or invalid JWT |
| `500` | Parse or database error |

---

## 9. System / Observability

These endpoints require no authentication.

### GET `/health`

Liveness check — returns 200 if the service is running.

#### Response `200 OK`

```jsonc
{
  "status":  "healthy",
  "service": "library-management-system",
  "version": "2.0"
}
```

#### TypeScript Interface

```ts
interface HealthResponse {
  status:  string;
  service: string;
  version: string;
}
```

---

### GET `/ready`

Readiness check — returns 200 when the service is ready to accept traffic.

#### Response `200 OK`

```jsonc
{
  "status": "ready"
}
```

#### TypeScript Interface

```ts
interface ReadyResponse {
  status: string;
}
```

---

### GET `/metrics`

Prometheus scrape endpoint. Returns metrics in Prometheus text exposition format.

**Content-Type:** `text/plain; version=0.0.4; charset=utf-8`

#### Response `200 OK`

```
# HELP http_requests_total Total HTTP requests
# TYPE http_requests_total counter
http_requests_total{method="POST",endpoint="/api/login"} 143
...
```

> Note: This endpoint is for monitoring systems (Prometheus, Grafana). Do not use in front-end code.

---

## 10. Data Types Reference

### Scalar Types Mapping

| C++ Type | JSON Type | TypeScript Type | Notes |
|----------|-----------|-----------------|-------|
| `int` | `number` | `number` | 32-bit integer |
| `long` | `number` | `number` | 64-bit integer — use `BigInt` for values > 2^53 |
| `size_t` | `number` | `number` | unsigned 64-bit — file sizes in bytes |
| `bool` | `boolean` | `boolean` | |
| `std::string` | `string` | `string` | |
| `std::optional<T>` | `T \| null` | `T \| null \| undefined` | Field may be absent |
| ISO timestamp | `string` | `string` | Format: `YYYY-MM-DDTHH:mm:ssZ` |

---

### User Roles

| Role Value | Description |
|------------|-------------|
| `"ADMIN"` | Full system access |
| `"LIBRARIAN"` | Manage media and users |
| `"MEMBER"` | Borrow and return media |
| `"STUDENT"` | Member with `grade_level` attribute |
| `"TEACHER"` | Member with `department` attribute |

> Note: `role` values are **case-insensitive** on input — the API normalises them to uppercase.

---

### CopyCondition Enum

Physical media copies have a condition field:

| Value | Description |
|-------|-------------|
| `"GOOD"` | No damage |
| `"FAIR"` | Minor wear |
| `"DAMAGED"` | Significant damage |
| `"LOST"` | Copy cannot be located |

---

### MediaType Values (media_type_id)

| ID | Type |
|----|------|
| `1` | Book |
| `2` | Magazine |
| `3` | DVD |
| `4` | AudioBook |
| `5` | DigitalMedia |

---

### Complete TypeScript Types Export

```ts
// ============================
// Roles & Enums
// ============================
export type UserRole = "ADMIN" | "LIBRARIAN" | "MEMBER" | "STUDENT" | "TEACHER";
export type CopyCondition = "GOOD" | "FAIR" | "DAMAGED" | "LOST";
export type MediaTypeId = 1 | 2 | 3 | 4 | 5;

// ============================
// Auth
// ============================
export interface LoginRequest {
  email:    string;
  password: string;
}
export interface LoginResponse {
  token: string;
}

// ============================
// Users
// ============================
export interface RegisterRequest {
  name:         string;
  email:        string;
  password:     string;
  role:         UserRole;
  grade_level?: string | null;
  department?:  string | null;
}
export interface RegisterResponse {
  message: string;
  user_id: number;
  role:    string;
}

export interface BaseUser {
  id:    number;
  name:  string;
  email: string;
  role:  string;
}
export interface StudentUser extends BaseUser { grade_level: string; }
export interface TeacherUser extends BaseUser { department: string; }
export type User = BaseUser | StudentUser | TeacherUser;
export type GetUsersResponse = User[];

export interface CreateUserRequest {
  name:         string;
  email:        string;
  password:     string;
  role?:        UserRole;
  grade_level?: string;
  department?:  string;
}
export interface CreateUserResponse { message: string; }

export interface AssignRoleRequest {
  user_id: number;
  role:    UserRole;
}
export interface AssignRoleResponse { message: string; }

// ============================
// Media
// ============================
export interface MediaItem {
  id:    number;
  title: string;
}
export type GetMediaResponse = MediaItem[];

export interface CreateBookRequest {
  title:          string;
  author:         string;
  isbn?:          string;
  media_type_id?: MediaTypeId;
}
export interface CreateBookResponse { message: string; }

// ============================
// Borrow & Return
// ============================
export interface BorrowRequest  { copy_id: number; }
export interface BorrowResponse { message: string; }

export interface ReturnRequest  { copy_id: number; }
export interface ReturnResponse { message: string; }

// ============================
// Search
// ============================
export interface SearchResult {
  id:    number;
  title: string;
  type?: string;
  [key: string]: unknown;
}
export interface SearchResponse {
  total:   number;
  from:    number;
  size:    number;
  results: SearchResult[];
}

export interface SuggestResponse {
  query:       string;
  suggestions: string[];
}

// ============================
// Digital Media
// ============================
export interface UploadDigitalMediaRequest {
  title:          string;
  mime_type:      string;
  file_data:      string;
  drm_protected?: boolean;
}
export interface DigitalMediaMetadata {
  media_id:        number;
  title:           string;
  mime_type:       string;
  s3_key:          string;
  file_size:       number;
  drm_protected:   boolean;
  current_version: number;
  created_at:      string;
}
export type UploadDigitalMediaResponse = DigitalMediaMetadata;

export interface DownloadUrlResponse {
  download_url: string;
  expires_in:   number;
}
export interface UploadUrlResponse {
  upload_url: string;
  media_id:   number;
}

export interface CreateVersionRequest  { file_data: string; }
export interface CreateVersionResponse {
  media_id:  number;
  version:   number;
  s3_key:    string;
  file_size: number;
}

export interface FileVersion {
  version_number: number;
  s3_key:         string;
  file_size:      number;
  checksum:       string;
  uploaded_at:    string;
  is_current:     boolean;
}
export interface ListVersionsResponse {
  media_id: number;
  versions: FileVersion[];
}

export interface DeleteDigitalMediaResponse { message: string; }

// ============================
// Batch Import
// ============================
export interface ImportRecord {
  title:   string;
  type?:   string;
  author?: string;
  isbn?:   string;
  [key: string]: unknown;
}
export interface ImportResponse {
  status:   "success" | "partial";
  total:    number;
  imported: number;
  failed:   number;
  errors:   string[];
}

// ============================
// System
// ============================
export interface HealthResponse {
  status:  string;
  service: string;
  version: string;
}
export interface ReadyResponse { status: string; }

// ============================
// Standard Error
// ============================
export interface ApiError { error: string; }
```

---

## 11. Error Responses

Every error response uses this consistent shape:

```ts
interface ApiError {
  error: string;   // human-readable error description
}
```

**Examples:**

```json
{ "error": "Missing email or password" }
{ "error": "Invalid email or password" }
{ "error": "Missing or invalid JWT" }
{ "error": "Only ADMIN users can assign roles" }
{ "error": "Digital media not found" }
```

**Suggested front-end error handling:**

```ts
async function apiFetch<T>(url: string, init?: RequestInit): Promise<T> {
  const res = await fetch(url, init);
  if (!res.ok) {
    const err: ApiError = await res.json();
    throw new Error(err.error);
  }
  return res.json() as Promise<T>;
}
```

---

## 12. Role & Permission Matrix

| Endpoint | No Auth | MEMBER / STUDENT / TEACHER | LIBRARIAN | ADMIN |
|----------|:-------:|:--------------------------:|:---------:|:-----:|
| `POST /api/login` | ✅ | ✅ | ✅ | ✅ |
| `POST /api/register` | ✅ | ✅ | ✅ | ✅ |
| `GET /api/media` | ✅ | ✅ | ✅ | ✅ |
| `GET /health` | ✅ | ✅ | ✅ | ✅ |
| `GET /ready` | ✅ | ✅ | ✅ | ✅ |
| `GET /metrics` | ✅ | ✅ | ✅ | ✅ |
| `GET /api/users` | ❌ | ✅ | ✅ | ✅ |
| `POST /api/borrow` | ❌ | ✅ | ✅ | ✅ |
| `POST /api/return` | ❌ | ✅ | ✅ | ✅ |
| `GET /api/search` | ❌ | ✅ | ✅ | ✅ |
| `GET /api/search/suggest` | ❌ | ✅ | ✅ | ✅ |
| `GET /api/digital-media/{id}` | ❌ | ✅ | ✅ | ✅ |
| `GET /api/digital-media/{id}/download-url` | ❌ | ✅ | ✅ | ✅ |
| `GET /api/digital-media/{id}/versions` | ❌ | ✅ | ✅ | ✅ |
| `POST /api/media/book` | ❌ | ❌ | ✅ | ✅ |
| `POST /api/digital-media/upload` | ❌ | ❌ | ✅ | ✅ |
| `GET /api/digital-media/{id}/upload-url` | ❌ | ❌ | ✅ | ✅ |
| `POST /api/digital-media/{id}/version` | ❌ | ❌ | ✅ | ✅ |
| `POST /api/import/json` | ❌ | ❌ | ✅ | ✅ |
| `POST /api/import/csv` | ❌ | ❌ | ✅ | ✅ |
| `POST /api/users` | ❌ | ❌ | ❌ | ✅ |
| `POST /api/users/assign-role` | ❌ | ❌ | ❌ | ✅ |
| `DELETE /api/digital-media/{id}` | ❌ | ❌ | ❌ | ✅ |

> ✅ = allowed  ❌ = denied (401 Unauthorized)
