#include "AuthService.h"

AuthService::AuthService(std::shared_ptr<PostgresAdapter> db,
                    std::shared_ptr<JwtHelper> jwt)
    : db_(std::move(db)), jwt_(std::move(jwt)) {}

std::optional<std::string> AuthService::login(const std::string& username,
                                    const std::string& password) {
    auto user = db_->getUserByName(username);
    std::cout << user->username << "\n" << user->role;
    if (!user.has_value()) return std::nullopt;
    if (!PasswordHasher::verify(password, user->hashedPassword))
        return std::nullopt;
    return jwt_->generateToken(user->id, user->role);
}

bool AuthService::verifyToken(const std::string& token) {
    return jwt_->verify(token);
}
