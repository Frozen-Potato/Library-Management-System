#pragma once
#include "User.h"

class Admin : public User {
public:
    Admin(int id, const std::string& name, const std::string& email)
        : User(id, name, email) {}
    std::string getRoleName() const override { return "Admin"; }
};
