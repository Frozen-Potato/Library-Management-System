#pragma once
#include "User.h"

class Member : public User {
public:
    Member(int id, const std::string& name, const std::string& email)
        : User(id, name, email) {}

    std::string getRoleName() const override { return "Member"; }
};