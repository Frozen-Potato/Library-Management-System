#include "Student.h"

std::string Student::getGrade(){
    return grade;
}

void Student::setGrade(std::string& gr){
    grade = std::move(gr);
}
