#ifndef TEACHER_H
#define TEACHER_H

#include "Member.h"

class Teacher: public Member {
    private:
        std::string department;
    public:
        using Member::Member;
        Teacher(int id, std::string name) : Member(id, std::move(name), 5) {}
        std::string getDepartment();
        void setDepartment(std::string& dep);
};

#endif