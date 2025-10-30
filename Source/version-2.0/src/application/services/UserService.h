#pragma once
#include <string>
#include <vector>
#include <memory>
#include "src/domain/user/User.h"
#include "src/data/PostgresAdapter.h"
#include "src/utils/Exceptions.h"

class UserService {
public:
    explicit UserService(std::shared_ptr<PostgresAdapter> db);

    int createUser( const std::string& name, const std::string& email,
                    const std::string& password, const std::string& role,
                    const std::optional<std::string>& gradeLevel,
                    const std::optional<std::string>& department);
    std::vector<std::shared_ptr<User>> listUsers();
    void assignRole(int userId, const std::string& role);

private:
    std::shared_ptr<PostgresAdapter> db_;
};
