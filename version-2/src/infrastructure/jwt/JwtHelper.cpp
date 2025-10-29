#include "src/infrastructure/jwt/JwtHelper.h"
#include <stdexcept>
#include <chrono>

JwtHelper::JwtHelper(const std::string& secret, const std::string& issuer, int expiresInMinutes)
    : secret_(secret), issuer_(issuer), expiresInMinutes_(expiresInMinutes) {}

std::string JwtHelper::generateToken(int userId, const std::string& role) const {
    auto now = std::chrono::system_clock::now();
    auto exp = now + std::chrono::minutes(expiresInMinutes_);

    auto token = jwt::create()
        .set_issuer(issuer_)
        .set_issued_at(now)
        .set_expires_at(exp)
        .set_payload_claim("uid", jwt::claim(std::to_string(userId)))
        .set_payload_claim("role", jwt::claim(role))
        .sign(jwt::algorithm::hs256{ secret_ });

    return token;
}

bool JwtHelper::verify(const std::string& token) const {
    try {
        auto decoded = jwt::decode(token);
        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{ secret_ })
            .with_issuer(issuer_)
            .verify(decoded);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string JwtHelper::getClaim(const std::string& token, const std::string& key) const {
    auto decoded = jwt::decode(token);
    if (decoded.has_payload_claim(key)) {
        return decoded.get_payload_claim(key).as_string();
    }
    throw std::runtime_error("Claim not found: " + key);
}
