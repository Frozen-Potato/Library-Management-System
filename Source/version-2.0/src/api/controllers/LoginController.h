#pragma once
#include <crow.h>
#include <memory>
#include "src/application/services/AuthService.h"
#include "src/api/middleware/JwtMiddleware.h"

class LoginController {
public:
    explicit LoginController(std::shared_ptr<AuthService> authService);
    void registerRoutes(crow::App<JwtMiddleware>& app);

private:
    std::shared_ptr<AuthService> authService_;
};
