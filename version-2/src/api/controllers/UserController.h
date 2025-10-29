#pragma once
#include <crow.h>
#include <memory>
#include "src/application/services/UserService.h"
#include "src/api/middleware/JwtMiddleware.h"

class UserController {
public:
    explicit UserController(std::shared_ptr<UserService> userService);
    void registerRoutes(crow::App<JwtMiddleware>& app);

private:
    std::shared_ptr<UserService> userService_;
};
