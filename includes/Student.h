#ifndef STUDENT_H
#define STUDENT_H

#include "Member.h"

class Student: public Member {
    private:
        std::string grade;
    public:
        using Member::Member;
        Student(int id, std::string name) : Member(id, std::move(name), 3) {}
        std::string getGrade();
        void setGrade(std::string& gr);
};

#endif