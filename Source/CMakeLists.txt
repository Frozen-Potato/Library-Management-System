cmake_minimum_required(VERSION 3.16)
project(LibrarySystem CXX)

# General Settings

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Include Directories

include_directories(
    ${PROJECT_SOURCE_DIR}/includes
    ${PROJECT_SOURCE_DIR}/includes/Interface
    ${PROJECT_SOURCE_DIR}/includes/db
    ${PROJECT_SOURCE_DIR}/includes/server
    ${PROJECT_SOURCE_DIR}/includes/external/cpp-httplib
    ${PROJECT_SOURCE_DIR}/schema
    ${PROJECT_SOURCE_DIR}/ultis
    /usr/local/include
    /usr/include/postgresql
)

# Source Files

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/ultis/*.cpp
    ${PROJECT_SOURCE_DIR}/src/db/*.cpp
    ${PROJECT_SOURCE_DIR}/main.cpp
    ${PROJECT_SOURCE_DIR}/src/server/*.cpp
    ${PROJECT_SOURCE_DIR}/schema/log_service.pb.cc
    ${PROJECT_SOURCE_DIR}/schema/log_service.grpc.pb.cc
)

# Library Paths

link_directories(
    /usr/local/lib
    /usr/lib/x86_64-linux-gnu
)

# PostgreSQL / libpqxx

find_package(PostgreSQL REQUIRED)
if (EXISTS "/usr/local/lib/libpqxx.so")
    message(STATUS "Using custom libpqxx from /usr/local/lib")
    set(PQXX_LIB /usr/local/lib/libpqxx.so)
else()
    message(WARNING "Falling back to system libpqxx")
    set(PQXX_LIB pqxx)
endif()

# MongoDB C++ Driver

find_package(PkgConfig REQUIRED)
pkg_check_modules(MONGOCXX REQUIRED libmongocxx)
pkg_check_modules(BSONCXX REQUIRED libbsoncxx)
include_directories(${MONGOCXX_INCLUDE_DIRS} ${BSONCXX_INCLUDE_DIRS})
link_directories(${MONGOCXX_LIBRARY_DIRS} ${BSONCXX_LIBRARY_DIRS})

# Platform Specifics

if (WIN32)
    message(STATUS "Building for Windows (MSYS2 detected)")
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    target_link_libraries(library_app PRIVATE ws2_32)
elseif(UNIX)
    message(STATUS "Building for Linux / WSL environment")
endif()

# REST API Server (cpp-httplib)

find_package(nlohmann_json REQUIRED)

# gRPC Server
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(PROTO_SRC
    ${PROJECT_SOURCE_DIR}/schema/log_service.pb.cc
    ${PROJECT_SOURCE_DIR}/schema/log_service.grpc.pb.cc
)

# Link final app

add_executable(library_server
    ${SOURCES}
)

target_include_directories(library_server PRIVATE
    ${CMAKE_SOURCE_DIR}/includes
    ${CMAKE_SOURCE_DIR}/schema
)

target_link_libraries(library_server
    PRIVATE
        pqxx pq
        mongocxx bsoncxx
        gRPC::grpc++
        gRPC::grpc
        gRPC::grpc++_reflection
        protobuf::libprotobuf
)


# Final Message

message(STATUS "Configuration complete: library_server")
