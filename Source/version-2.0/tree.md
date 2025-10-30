```
.
├── CMakeLists.txt
├── build
│   ├── CMakeCache.txt
│   ├── CMakeFiles
│   │   ├── 3.28.3
│   │   │   ├── CMakeCXXCompiler.cmake
│   │   │   ├── CMakeDetermineCompilerABI_CXX.bin
│   │   │   ├── CMakeSystem.cmake
│   │   │   └── CompilerIdCXX
│   │   │       ├── CMakeCXXCompilerId.cpp
│   │   │       ├── a.out
│   │   │       └── tmp
│   │   ├── CMakeConfigureLog.yaml
│   │   ├── CMakeDirectoryInformation.cmake
│   │   ├── CMakeScratch
│   │   ├── Makefile.cmake
│   │   ├── Makefile2
│   │   ├── TargetDirectories.txt
│   │   ├── cmake.check_cache
│   │   ├── library_server.dir
│   │   │   ├── DependInfo.cmake
│   │   │   ├── build.make
│   │   │   ├── cmake_clean.cmake
│   │   │   ├── compiler_depend.make
│   │   │   ├── compiler_depend.ts
│   │   │   ├── depend.make
│   │   │   ├── flags.make
│   │   │   ├── link.txt
│   │   │   ├── main.cpp.o
│   │   │   ├── main.cpp.o.d
│   │   │   ├── progress.make
│   │   │   ├── proto
│   │   │   │   ├── log_service.grpc.pb.cc.o
│   │   │   │   ├── log_service.grpc.pb.cc.o.d
│   │   │   │   ├── log_service.pb.cc.o
│   │   │   │   └── log_service.pb.cc.o.d
│   │   │   └── src
│   │   │       ├── api
│   │   │       │   ├── controllers
│   │   │       │   │   ├── BorrowController.cpp.o
│   │   │       │   │   ├── BorrowController.cpp.o.d
│   │   │       │   │   ├── LoginController.cpp.o
│   │   │       │   │   ├── LoginController.cpp.o.d
│   │   │       │   │   ├── MediaController.cpp.o
│   │   │       │   │   ├── MediaController.cpp.o.d
│   │   │       │   │   ├── ReturnController.cpp.o
│   │   │       │   │   ├── ReturnController.cpp.o.d
│   │   │       │   │   ├── UserController.cpp.o
│   │   │       │   │   └── UserController.cpp.o.d
│   │   │       │   ├── grpc
│   │   │       │   │   ├── LogServiceServer.cpp.o
│   │   │       │   │   └── LogServiceServer.cpp.o.d
│   │   │       │   └── middleware
│   │   │       │       ├── JwtMiddleware.cpp.o
│   │   │       │       └── JwtMiddleware.cpp.o.d
│   │   │       ├── application
│   │   │       │   └── services
│   │   │       │       ├── AuthService.cpp.o
│   │   │       │       ├── AuthService.cpp.o.d
│   │   │       │       ├── LibraryService.cpp.o
│   │   │       │       ├── LibraryService.cpp.o.d
│   │   │       │       ├── PgQueueService.cpp.o
│   │   │       │       ├── PgQueueService.cpp.o.d
│   │   │       │       ├── UserService.cpp.o
│   │   │       │       └── UserService.cpp.o.d
│   │   │       ├── data
│   │   │       │   ├── MongoAdapter.cpp.o
│   │   │       │   ├── MongoAdapter.cpp.o.d
│   │   │       │   ├── PostgresAdapter.cpp.o
│   │   │       │   └── PostgresAdapter.cpp.o.d
│   │   │       └── infrastructure
│   │   │           ├── config
│   │   │           │   ├── ConfigManager.cpp.o
│   │   │           │   ├── ConfigManager.cpp.o.d
│   │   │           │   ├── EnvLoader.cpp.o
│   │   │           │   └── EnvLoader.cpp.o.d
│   │   │           ├── crypto
│   │   │           │   ├── PasswordHasher.cpp.o
│   │   │           │   └── PasswordHasher.cpp.o.d
│   │   │           ├── db
│   │   │           │   ├── MongoConnection.cpp.o
│   │   │           │   ├── MongoConnection.cpp.o.d
│   │   │           │   ├── PostgresPool.cpp.o
│   │   │           │   └── PostgresPool.cpp.o.d
│   │   │           ├── jwt
│   │   │           │   ├── JwtHelper.cpp.o
│   │   │           │   └── JwtHelper.cpp.o.d
│   │   │           └── queue
│   │   │               ├── PersistentQueue.cpp.o
│   │   │               ├── PersistentQueue.cpp.o.d
│   │   │               ├── QueueWorker.cpp.o
│   │   │               └── QueueWorker.cpp.o.d
│   │   ├── pkgRedirects
│   │   └── progress.marks
│   ├── Makefile
│   ├── cmake_install.cmake
│   └── library_server
├── build.sh
├── db
│   ├── schema.sql
│   └── seed.sql
├── docker
│   ├── Dockerfile
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
│   │       └── JwtMiddleware.h
│   ├── application
│   │   └── services
│   │       ├── AuthService.cpp
│   │       ├── AuthService.h
│   │       ├── LibraryService.cpp
│   │       ├── LibraryService.h
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
├── test
└── tree.md

47 directories, 150 files
```