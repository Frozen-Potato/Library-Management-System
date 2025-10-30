#pragma once
#include "Member.h"

class Student : public Member {
public:
    Student(int id, const std::string& name, const std::string& email, std::string grade)
        : Member(id, name, email), grade_(std::move(grade)) {}

    const std::string& getGrade() const { return grade_; }
    std::string getRoleName() const override { return "Student"; }

private:
    std::string grade_;
};
