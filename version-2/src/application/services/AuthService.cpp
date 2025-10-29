#include "AuthService.h"
#include "src/infrastructure/crypto/PasswordHasher.h"
#include "src/infrastructure/jwt/JwtHelper.h"
#include "src/data/PostgresAdapter.h"

class DefaultAuthService : public AuthService {
public:
    DefaultAuthService(std::shared_ptr<PostgresAdapter> db,
                       std::shared_ptr<JwtHelper> jwt)
        : db_(std::move(db)), jwt_(std::move(jwt)) {}

    std::optional<std::string> login(const std::string& username,
                                     const std::string& password) override {
        auto user = db_->getUserByName(username);
        if (!user.has_value()) return std::nullopt;
        if (!PasswordHasher::verify(password, user->hashedPassword))
            return std::nullopt;
        return jwt_->generateToken(user->id, user->role);
    }

    bool verifyToken(const std::string& token) override {
        return jwt_->verify(token);
    }

private:
    std::shared_ptr<PostgresAdapter> db_;
    std::shared_ptr<JwtHelper> jwt_;
};
