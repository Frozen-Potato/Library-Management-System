#include "UserService.h"
#include "src/infrastructure/crypto/PasswordHasher.h"

UserService::UserService(std::shared_ptr<PostgresAdapter> db)
    : db_(std::move(db)) {}

int UserService::createUser(const std::string& name, const std::string& email,
                            const std::string& password, const std::string& role) {
    std::string hashed = PasswordHasher::hash(password);
    return db_->insertUser(name, email, hashed, role);
}

std::vector<std::shared_ptr<User>> UserService::listUsers() {
    return db_->getAllUsers();
}

void UserService::assignRole(int userId, const std::string& role) {
    db_->assignRole(userId, role);
}
