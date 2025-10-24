#ifndef EXCEPT_H
#define EXCEPT_H

#include <stdexcept>
#include <string>

class BorrowLimitExceeded : public std::runtime_error {
    public: BorrowLimitExceeded(const std::string& msg) : std::runtime_error("Borrow limit exceeded: " + msg) {}
};

class BookUnavailable : public std::runtime_error {
    public: BookUnavailable(const std::string& msg) : std::runtime_error("Book Unavailable: " + msg) {};
};

class GeneralFailure : public std::runtime_error {
    public: GeneralFailure(const std::string& msg) : std::runtime_error(msg) {};
};

class DuplicationException : public std::runtime_error {
    public: DuplicationException(const std::string& msg) : std::runtime_error("Duplication Error: " + msg) {};
}; 

#endif