#include "src/api/controllers/LoginController.h"
#include "src/utils/JsonUtils.h"
#include "src/utils/Exceptions.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

LoginController::LoginController(std::shared_ptr<AuthService> authService)
    : authService_(std::move(authService)) {}

void LoginController::registerRoutes(crow::App<JwtMiddleware>& app) {
    CROW_ROUTE(app, "/api/login").methods(crow::HTTPMethod::POST)(
        [this](const crow::request& req) {
            try {
                auto body = parseJsonSafe(req.body);

                if (!body.contains("email") || !body.contains("password"))
                    throw ValidationException("Missing email or password");

                auto token = authService_->login(body["email"], body["password"]);
                if (!token.has_value())
                    throw UnauthorizedException("Invalid email or password");

                json resp = {{"token", token.value()}};
                return crow::response(200, resp.dump());
            }
            catch (const ValidationException& e) { return crow::response(400, makeJsonError(e.what())); }
            catch (const UnauthorizedException& e) { return crow::response(401, makeJsonError(e.what())); }
            catch (const std::exception& e) { return crow::response(500, makeJsonError(e.what())); }
        });
}
