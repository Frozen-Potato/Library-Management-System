#pragma once
#include <stdexcept>
#include <string>

class NotFoundException : public std::runtime_error {
public:
    explicit NotFoundException(const std::string& msg)
        : std::runtime_error("Not Found: " + msg) {}
};

class ValidationException : public std::runtime_error {
public:
    explicit ValidationException(const std::string& msg)
        : std::runtime_error("Validation Error: " + msg) {}
};

class DatabaseException : public std::runtime_error {
public:
    explicit DatabaseException(const std::string& msg)
        : std::runtime_error("Database Error: " + msg) {}
};

class UnauthorizedException : public std::runtime_error {
public:
    explicit UnauthorizedException(const std::string& msg)
        : std::runtime_error("Unauthorized: " + msg) {}
};
