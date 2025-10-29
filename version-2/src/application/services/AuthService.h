#pragma once
#include <string>
#include <optional>

class AuthService {
public:
    virtual ~AuthService() = default;
    virtual std::optional<std::string> login(const std::string& username,
                                             const std::string& password) = 0;
    virtual bool verifyToken(const std::string& token) = 0;
};
