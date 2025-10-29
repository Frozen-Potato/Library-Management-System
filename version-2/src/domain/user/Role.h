#pragma once
#include <string>
#include <vector>
#include "Permission.h"

struct Role {
    long id;
    std::string name;
    std::vector<Permission> permissions;
};
