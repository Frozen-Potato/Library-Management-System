#pragma once
#include "Member.h"

class Librarian : public Member {
public:
    Librarian(int id, const std::string& name, const std::string& email)
        : Member(id, name, email) {}
    std::string getRoleName() const override { return "Librarian"; }
};
