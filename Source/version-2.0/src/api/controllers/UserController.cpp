#include "src/api/controllers/UserController.h"
#include "src/utils/JsonUtils.h"
#include "src/utils/Exceptions.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

UserController::UserController(std::shared_ptr<UserService> userService)
    : userService_(std::move(userService)) {}

void UserController::registerRoutes(crow::App<JwtMiddleware>& app) {

    // Register
    CROW_ROUTE(app, "/api/register").methods(crow::HTTPMethod::POST)(
        [this](const crow::request& req) {
            try {
                auto body = parseJsonSafe(req.body);

                if (!body.contains("name") || !body.contains("email") ||
                    !body.contains("password") || !body.contains("role")) {
                    throw ValidationException("Missing required fields: name, email, password, or role");
                }

                std::string name = body["name"];
                std::string email = body["email"];
                std::string password = body["password"];
                std::string role = body["role"];

                std::optional<std::string> gradeLevel = std::nullopt;
                std::optional<std::string> department = std::nullopt;

                if (body.contains("grade_level") && !body["grade_level"].is_null())
                    gradeLevel = body["grade_level"].get<std::string>();
                if (body.contains("department") && !body["department"].is_null())
                    department = body["department"].get<std::string>();

                int userId = userService_->createUser(name, email, password, role, gradeLevel, department);

                json resp = {
                    {"message", "User registered successfully"},
                    {"user_id", userId},
                    {"role", role}
                };

                return crow::response(201, resp.dump());
            }
            catch (const ValidationException& e) { return crow::response(400, makeJsonError(e.what())); }
            catch (const DatabaseException& e) { return crow::response(500, makeJsonError(e.what())); }
            catch (const std::exception& e) { return crow::response(500, makeJsonError(e.what())); }
        });

    // GET /api/users
    CROW_ROUTE(app, "/api/users").methods(crow::HTTPMethod::GET)(
        [this, &app](const crow::request& req, crow::response& res) {
            try {
                const auto& ctx = app.get_context<JwtMiddleware>(req);
                if (!ctx.valid)
                    throw UnauthorizedException("Missing or invalid JWT");

                auto users = userService_->listUsers();
                json result = json::array();
                for (auto& u : users) {
                json userJson = {
                    {"id", u->getId()},
                    {"name", u->getName()},
                    {"email", u->getEmail()},
                    {"role", u->getRoleName()},
                };

                // Add subclass-specific details
                if (auto student = dynamic_cast<Student*>(u.get())) {
                    userJson["grade_level"] = student->getGrade();
                } 
                else if (auto teacher = dynamic_cast<Teacher*>(u.get())) {
                    userJson["department"] = teacher->getDepartment();
                }

                result.push_back(userJson);
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
    [this, &app](const crow::request& req, crow::response& res) {
        try {
            const auto& ctx = app.get_context<JwtMiddleware>(req);
            if (!ctx.valid)
                throw UnauthorizedException("Missing or invalid JWT");

            auto body = parseJsonSafe(req.body);

            // --- Validate required fields ---
            if (!body.contains("name") || !body.contains("email") || !body.contains("password"))
                throw ValidationException("Missing required fields: name, email, or password");

            // --- Extract role (default to Member if not specified) ---
            std::string role = body.value("role", "Member");

            // --- Extract optional Student/Teacher attributes ---
            std::optional<std::string> gradeLevel = std::nullopt;
            std::optional<std::string> department = std::nullopt;

            if (body.contains("grade_level") && body["grade_level"].is_string()) {
                gradeLevel = body["grade_level"].get<std::string>();
            }
            if (body.contains("department") && body["department"].is_string()) {
                department = body["department"].get<std::string>();
            }

            // --- Create user through service layer ---
            int userId = userService_->createUser(
                body["name"], body["email"], body["password"], role, gradeLevel, department
            );

            res.code = 201;
            res.write(makeJsonSuccess("User created with ID " + std::to_string(userId)));
        }
        catch (const ValidationException& e) {
            res.code = 400;
            res.write(makeJsonError(e.what()));
        }
        catch (const UnauthorizedException& e) {
            res.code = 401;
            res.write(makeJsonError(e.what()));
        }
        catch (const DatabaseException& e) {
            res.code = 500;
            res.write(makeJsonError(e.what()));
        }
        catch (const std::exception& e) {
            res.code = 500;
            res.write(makeJsonError(e.what()));
        }

        res.end();
    });


    // POST /api/users/assign-role
    CROW_ROUTE(app, "/api/users/assign-role").methods(crow::HTTPMethod::POST)(
        [this, &app](const crow::request& req, crow::response& res) {
            try {
                const auto& ctx = app.get_context<JwtMiddleware>(req);
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
