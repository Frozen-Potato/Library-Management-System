#pragma once
#include "Member.h"

class Teacher : public Member {
public:
    Teacher(int id, const std::string& name, const std::string& email, std::string department)
        : Member(id, name, email), department_(std::move(department)) {}

    const std::string& getDepartment() const { return department_; }
    std::string getRoleName() const override { return "Teacher"; }

private:
    std::string department_;
};
