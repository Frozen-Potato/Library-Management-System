#include "src/api/middleware/JwtMiddleware.h"
#include <jwt-cpp/jwt.h>

void JwtMiddleware::before_handle(crow::request& req, crow::response& res, context& ctx) {
    if (req.url.find("/api/login") == 0 || req.url.find("/api/media") == 0) {
        ctx.valid = false;
        return;
    }

    auto authHeader = req.get_header_value("Authorization");
    if (authHeader.empty() || authHeader.rfind("Bearer ", 0) != 0) {
        res.code = 401;
        res.write("Unauthorized: Missing Bearer token");
        res.end();
        return;
    }

    std::string token = authHeader.substr(7);
    if (!jwt_->verify(token)) {
        res.code = 401;
        res.write("Unauthorized: Invalid or expired token");
        res.end();
        return;
    }

    try {
        ctx.userId = jwt_->getClaim(token, "uid");
        ctx.role = jwt_->getClaim(token, "role");
        ctx.valid = true;
    } catch (...) {
        ctx.valid = false;
        res.code = 401;
        res.write("Unauthorized: Token missing claims");
        res.end();
    }
}
