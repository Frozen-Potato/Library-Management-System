#include "Teacher.h"

std::string Teacher::getDepartment(){
    return department;
}

void Teacher::setDepartment(std::string& dep){
    department = std::move(dep);
}