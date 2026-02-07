cmake_minimum_required(VERSION 3.16)
project(LibrarySystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Dependencies (System + Custom Includes)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

find_package(PostgreSQL REQUIRED)
if (EXISTS "/usr/local/lib/libpqxx.so")
    message(STATUS "Using custom libpqxx from /usr/local/lib")
    set(PQXX_LIB /usr/local/lib/libpqxx.so)
else()
    message(WARNING "Falling back to system libpqxx")
    set(PQXX_LIB pqxx)
endif()

pkg_check_modules(LIBPQ REQUIRED libpq)
pkg_check_modules(MONGOCXX REQUIRED libmongocxx)
pkg_check_modules(BSONCXX REQUIRED libbsoncxx)
include_directories(${MONGOCXX_INCLUDE_DIRS} ${BSONCXX_INCLUDE_DIRS})
link_directories(${MONGOCXX_LIBRARY_DIRS} ${BSONCXX_LIBRARY_DIRS})


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Protobuf_INCLUDE_DIRS}
    ${LIBPQ_INCLUDE_DIRS}
    ${MONGOCXX_INCLUDE_DIRS}
    ${BSONCXX_INCLUDE_DIRS}
    /usr/local/include
    /usr/include
)

link_directories(
    ${LIBPQ_LIBRARY_DIRS}
    ${MONGOCXX_LIBRARY_DIRS}
    ${BSONCXX_LIBRARY_DIRS}
    /usr/local/lib
    /usr/lib/x86_64-linux-gnu
)


# Proto / gRPC

set(PROTO_SRC
    proto/log_service.pb.cc
    proto/log_service.grpc.pb.cc
)
set(PROTO_HDR
    proto/log_service.pb.h
    proto/log_service.grpc.pb.h
)


# Collect all source files explicitly
file(GLOB_RECURSE SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# Debug: Print all source files being compiled
message(STATUS "Source files: ${SRC_FILES}")


# Build Target

add_executable(library_server
    ${SRC_FILES}
    ${PROTO_SRC}
)



# Libraries to Link

target_link_libraries(library_server
    PRIVATE
        ${LIBPQ_LIBRARIES}
        ${MONGOCXX_LIBRARIES}
        ${BSONCXX_LIBRARIES}
        ${Protobuf_LIBRARIES}
        pthread
        ssl
        crypto
        pqxx pq
        mongocxx bsoncxx
        gRPC::grpc++
        gRPC::grpc
        gRPC::grpc++_reflection
        protobuf::libprotobuf
        gpr
        address_sorting
        bcrypt
        hiredis
        rdkafka
        rdkafka++
        curl
)


# Include directories for headers

target_include_directories(library_server PRIVATE
    src
    proto
    /usr/local/include
)


# Output Directory

set_target_properties(library_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Test

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(run_tests
    tests/test_main.cpp
    tests/test_user_service.cpp
    tests/test_auth_service.cpp

    src/application/services/UserService.cpp
    src/application/services/AuthService.cpp
    src/data/PostgresAdapter.cpp
    src/infrastructure/jwt/JwtHelper.cpp
    src/infrastructure/crypto/PasswordHasher.cpp
)

target_link_libraries(run_tests
    PRIVATE
        gtest_main
        pqxx
        pq       
        pthread
        crypto
        ssl
        bcrypt
)

add_test(NAME run_tests COMMAND run_tests)

include(GoogleTest)
gtest_discover_tests(run_tests)