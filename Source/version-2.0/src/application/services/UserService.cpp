#include "UserService.h"
#include "src/infrastructure/crypto/PasswordHasher.h"

UserService::UserService(std::shared_ptr<PostgresAdapter> db)
    : db_(std::move(db)) {}

int UserService::createUser(const std::string& name,
                            const std::string& email,
                            const std::string& password,
                            const std::string& role,
                            const std::optional<std::string>& gradeLevel,
                            const std::optional<std::string>& department) {
    std::string hashed = PasswordHasher::hash(password);

    try {
        return db_->insertUser(name, email, hashed, role, gradeLevel, department);
    } catch (const pqxx::sql_error& e) {
        throw DatabaseException(std::string("Database error: ") + e.what());
    } catch (const std::exception& e) {
        throw DatabaseException(std::string("Unexpected error: ") + e.what());
    }
}


std::vector<std::shared_ptr<User>> UserService::listUsers() {
    return db_->getAllUsers();
}

void UserService::assignRole(int userId, const std::string& role) {
    db_->assignRole(userId, role);
}
