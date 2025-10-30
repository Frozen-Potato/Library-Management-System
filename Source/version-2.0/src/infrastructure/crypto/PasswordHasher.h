#pragma once
#include <string>

class PasswordHasher {
public:
    // Hash a plain text password using bcrypt (or SHA256 fallback)
    static std::string hash(const std::string& password);

    // Verify a plain text password against the stored hash
    static bool verify(const std::string& password, const std::string& hash);
};