#pragma once
#include <crow.h>
#include "src/infrastructure/jwt/JwtHelper.h"

class JwtMiddleware {
public:
    struct context {
        std::string userId;
        std::string role;
        bool valid = false;
    };

    JwtMiddleware() = default;
    JwtMiddleware(std::shared_ptr<JwtHelper> jwt) : jwt_(std::move(jwt)) {}

    void before_handle(crow::request& req, crow::response& res, context& ctx);
    void after_handle(crow::request&, crow::response&, context&) {}

private:
    std::shared_ptr<JwtHelper> jwt_;
};
