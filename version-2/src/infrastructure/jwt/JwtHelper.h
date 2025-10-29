#pragma once
#include <string>
#include <jwt-cpp/jwt.h>

class JwtHelper {
public:
    JwtHelper(const std::string& secret, const std::string& issuer = "library-system",
              int expiresInMinutes = 60);

    // Create token for user
    std::string generateToken(int userId, const std::string& role) const;

    // Verify token and return whether it’s valid
    bool verify(const std::string& token) const;

    // Extract claim from token (optional helper)
    std::string getClaim(const std::string& token, const std::string& key) const;

private:
    std::string secret_;
    std::string issuer_;
    int expiresInMinutes_;
};