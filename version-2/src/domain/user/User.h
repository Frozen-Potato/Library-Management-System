#pragma once
#include <string>
#include <memory>

class User {
public:
    User(int id, std::string name, std::string email)
        : id_(id), name_(std::move(name)), email_(std::move(email)) {}
    virtual ~User() = default;

    int getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getEmail() const { return email_; }

    virtual std::string getRoleName() const = 0;

protected:
    int id_;
    std::string name_;
    std::string email_;
};
