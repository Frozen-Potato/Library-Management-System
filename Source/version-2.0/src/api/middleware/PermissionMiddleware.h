#pragma once
#include <crow.h>
#include <memory>
#include "src/api/middleware/JwtMiddleware.h"
#include "src/application/services/PermissionService.h"

class PermissionMiddleware {
public:
    struct context {
        bool allowed = false;
    };

    explicit PermissionMiddleware(std::shared_ptr<PermissionService> permissionService)
        : permissionService_(std::move(permissionService)) {}

    void before_handle(crow::request& req, crow::response& res, context& ctx);
    void after_handle(crow::request&, crow::response&, context&) {}

private:
    std::shared_ptr<PermissionService> permissionService_;

    static std::string extractTableName(const std::string& url);
    static std::string deduceAction(const crow::request& req);
};
