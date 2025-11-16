#include "src/api/middleware/PermissionMiddleware.h"
#include <iostream>
#include <regex>

void PermissionMiddleware::before_handle(crow::request& req, crow::response& res, context& ctx) {
    // Retrieve JWT context from previous middleware
    auto* jwtCtx = req.template get_context<JwtMiddleware::context>();

    if (!jwtCtx) {
        res.code = 401;
        res.write("Unauthorized: Invalid JWT context");
        res.end();
        return;
    }

    if (jwtCtx.isPublic) {
        ctx.allowed = true;
        return;
    }

    if (!jwtCtx.valid) {
        res.code = 401;
        res.write("Unauthorized: missing or invalid JWT");
        res.end();
        return;
    }

    // Derive table and action
    std::string table = extractTableName(req.url);
    std::string action = deduceAction(req);

    if (table.empty()) {
        res.code = 400;
        res.write("Bad Request: Cannot determine target table");
        res.end();
        return;
    }

    // Check permission
    bool permitted = permissionService_->checkPermission(jwtCtx.role, table, action);
    if (!permitted) {
        res.code = 403;
        res.write("Forbidden: " + jwtCtx.role + " cannot " + action + " on " + table);
        res.end();
        return;
    }

    ctx.allowed = true;
}

std::string PermissionMiddleware::extractTableName(const std::string& url) {
    std::regex pattern(R"(/api/([a-zA-Z_]+))");
    std::smatch match;
    if (std::regex_search(url, match, pattern) && match.size() > 1)
        return match[1];
    return {};
}

std::string PermissionMiddleware::deduceAction(const crow::request& req) {
    using namespace crow;
    switch (req.method) {
        case HTTPMethod::GET:    return "read";
        case HTTPMethod::POST:   return "write";
        case HTTPMethod::PUT:    return "write";
        case HTTPMethod::DELETE: return "write";
        default: return "read";
    }
}
