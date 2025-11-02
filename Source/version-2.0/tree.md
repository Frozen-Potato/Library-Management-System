├── CMakeLists.txt
├── build.sh
├── db
│   ├── schema.sql
│   └── seed.sql
├── docker
│   ├── Dockerfile
│   ├── Dockerfile.test
│   └── docker-compose.yml
├── main.cpp
├── proto
│   ├── log_service.grpc.pb.cc
│   ├── log_service.grpc.pb.h
│   ├── log_service.pb.cc
│   ├── log_service.pb.h
│   └── log_service.proto
├── src
│   ├── api
│   │   ├── controllers
│   │   │   ├── BorrowController.cpp
│   │   │   ├── BorrowController.h
│   │   │   ├── LoginController.cpp
│   │   │   ├── LoginController.h
│   │   │   ├── MediaController.cpp
│   │   │   ├── MediaController.h
│   │   │   ├── ReturnController.cpp
│   │   │   ├── ReturnController.h
│   │   │   ├── UserController.cpp
│   │   │   └── UserController.h
│   │   ├── grpc
│   │   │   ├── LogServiceServer.cpp
│   │   │   └── LogServiceServer.h
│   │   └── middleware
│   │       ├── JwtMiddleware.cpp
│   │       ├── JwtMiddleware.h
│   │       ├── PermissionMiddleware.cpp
│   │       └── PermissionMiddleware.h
│   ├── application
│   │   └── services
│   │       ├── AuthService.cpp
│   │       ├── AuthService.h
│   │       ├── LibraryService.cpp
│   │       ├── LibraryService.h
│   │       ├── PermissionService.h
│   │       ├── PgQueueService.cpp
│   │       ├── PgQueueService.h
│   │       ├── QueueService.h
│   │       ├── UserService.cpp
│   │       └── UserService.h
│   ├── data
│   │   ├── MongoAdapter.cpp
│   │   ├── MongoAdapter.h
│   │   ├── PostgresAdapter.cpp
│   │   └── PostgresAdapter.h
│   ├── domain
│   │   ├── borrow
│   │   │   ├── ActiveBorrow.h
│   │   │   ├── BorrowHistory.h
│   │   │   └── BorrowRecord.h
│   │   ├── media
│   │   │   ├── AudioBook.h
│   │   │   ├── Book.h
│   │   │   ├── DVD.h
│   │   │   ├── Magazine.h
│   │   │   ├── Media.h
│   │   │   └── MediaCopy.h
│   │   └── user
│   │       ├── Admin.h
│   │       ├── Librarian.h
│   │       ├── Member.h
│   │       ├── Permission.h
│   │       ├── Role.h
│   │       ├── Student.h
│   │       ├── Teacher.h
│   │       └── User.h
│   ├── infrastructure
│   │   ├── config
│   │   │   ├── ConfigManager.cpp
│   │   │   ├── ConfigManager.h
│   │   │   ├── EnvLoader.cpp
│   │   │   └── EnvLoader.h
│   │   ├── crypto
│   │   │   ├── PasswordHasher.cpp
│   │   │   └── PasswordHasher.h
│   │   ├── db
│   │   │   ├── MongoConnection.cpp
│   │   │   ├── MongoConnection.h
│   │   │   ├── PostgresPool.cpp
│   │   │   └── PostgresPool.h
│   │   ├── jwt
│   │   │   ├── JwtHelper.cpp
│   │   │   └── JwtHelper.h
│   │   └── queue
│   │       ├── PersistentQueue.cpp
│   │       ├── PersistentQueue.h
│   │       ├── QueueWorker.cpp
│   │       └── QueueWorker.h
│   └── utils
│       ├── DateTimeUtils.h
│       ├── Exceptions.h
│       ├── JsonUtils.h
│       └── StringUtils.h
├── tests
│   ├── test_auth_service.cpp
│   ├── test_main.cpp
│   └── test_user_service.cpp