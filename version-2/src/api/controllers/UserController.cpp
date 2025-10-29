#include "src/api/controllers/UserController.h"
#include "src/utils/JsonUtils.h"
#include "src/utils/Exceptions.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

UserController::UserController(std::shared_ptr<UserService> userService)
    : userService_(std::move(userService)) {}

void UserController::registerRoutes(crow::App<JwtMiddleware>& app) {

    // GET /api/users
    CROW_ROUTE(app, "/api/users").methods(crow::HTTPMethod::GET)(
        [this](const crow::request&, crow::response& res, const JwtMiddleware::context& ctx) {
            try {
                if (!ctx.valid)
                    throw UnauthorizedException("Missing or invalid JWT");

                auto users = userService_->listUsers();
                json result = json::array();
                for (auto& u : users) {
                    result.push_back({
                        {"id", u->getId()},
                        {"name", u->getName()},
                        {"email", u->getEmail()},
                        {"role", u->getRoleName()}
                    });
                }

                res.code = 200;
                res.write(result.dump());
            }
            catch (const UnauthorizedException& e) { res.code = 401; res.write(makeJsonError(e.what())); }
            catch (const DatabaseException& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            catch (const std::exception& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            res.end();
        });

    // POST /api/users
    CROW_ROUTE(app, "/api/users").methods(crow::HTTPMethod::POST)(
        [this](const crow::request& req, crow::response& res, const JwtMiddleware::context& ctx) {
            try {
                if (!ctx.valid)
                    throw UnauthorizedException("Missing or invalid JWT");

                auto body = parseJsonSafe(req.body);
                if (!body.contains("name") || !body.contains("email") || !body.contains("password"))
                    throw ValidationException("Missing required fields: name, email, or password");

                std::string role = body.value("role", "MEMBER");
                int userId = userService_->createUser(
                    body["name"], body["email"], body["password"], role
                );

                res.code = 201;
                res.write(makeJsonSuccess("User created with ID " + std::to_string(userId)));
            }
            catch (const ValidationException& e) { res.code = 400; res.write(makeJsonError(e.what())); }
            catch (const UnauthorizedException& e) { res.code = 401; res.write(makeJsonError(e.what())); }
            catch (const DatabaseException& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            catch (const std::exception& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            res.end();
        });

    // POST /api/users/assign-role
    CROW_ROUTE(app, "/api/users/assign-role").methods(crow::HTTPMethod::POST)(
        [this](const crow::request& req, crow::response& res, const JwtMiddleware::context& ctx) {
            try {
                if (!ctx.valid)
                    throw UnauthorizedException("Missing or invalid JWT");
                if (ctx.role != "ADMIN")
                    throw UnauthorizedException("Only ADMIN users can assign roles");

                auto body = parseJsonSafe(req.body);
                if (!body.contains("user_id") || !body.contains("role"))
                    throw ValidationException("Missing user_id or role");

                userService_->assignRole(body["user_id"], body["role"]);

                res.code = 200;
                res.write(makeJsonSuccess("Role assigned successfully"));
            }
            catch (const ValidationException& e) { res.code = 400; res.write(makeJsonError(e.what())); }
            catch (const UnauthorizedException& e) { res.code = 401; res.write(makeJsonError(e.what())); }
            catch (const NotFoundException& e) { res.code = 404; res.write(makeJsonError(e.what())); }
            catch (const DatabaseException& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            catch (const std::exception& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            res.end();
        });
}
