#pragma once
#include <string>
#include <optional>
#include "src/infrastructure/crypto/PasswordHasher.h"
#include "src/infrastructure/jwt/JwtHelper.h"
#include "src/data/PostgresAdapter.h"

class DefaultAuthService {
public:
    virtual ~DefaultAuthService() = default;
    virtual std::optional<std::string> login(const std::string& username,
                                             const std::string& password) = 0;
    virtual bool verifyToken(const std::string& token) = 0;
};

class AuthService : public DefaultAuthService {
public:
    AuthService(std::shared_ptr<PostgresAdapter> db,
                       std::shared_ptr<JwtHelper> jwt);

    std::optional<std::string> login(const std::string& username,
                                     const std::string& password) override;

    bool verifyToken(const std::string& token) override;

private:
    std::shared_ptr<PostgresAdapter> db_;
    std::shared_ptr<JwtHelper> jwt_;
};