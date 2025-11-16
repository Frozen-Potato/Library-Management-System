#include "src/api/controllers/BorrowController.h"
#include "src/utils/JsonUtils.h"
#include "src/utils/Exceptions.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

BorrowController::BorrowController(std::shared_ptr<LibraryService> service)
    : service_(std::move(service)) {}

void BorrowController::registerRoutes(crow::App<JwtMiddleware, PermissionMiddleware>& app) {
    CROW_ROUTE(app, "/api/borrow").methods(crow::HTTPMethod::POST)(
        [this, &app](const crow::request& req, crow::response& res) {
            try {
                const auto& ctx = app.get_context<JwtMiddleware>(req);

                if (!ctx.valid)
                    throw UnauthorizedException("Missing or invalid JWT");

                auto body = parseJsonSafe(req.body);
                if (!body.contains("copy_id"))
                    throw ValidationException("Missing copy_id");

                service_->borrowCopy(std::stoi(ctx.userId), body["copy_id"]);

                res.code = 200;
                res.write(makeJsonSuccess("Borrow successful"));
            }
            catch (const ValidationException& e) { res.code = 400; res.write(makeJsonError(e.what())); }
            catch (const UnauthorizedException& e) { res.code = 401; res.write(makeJsonError(e.what())); }
            catch (const NotFoundException& e) { res.code = 404; res.write(makeJsonError(e.what())); }
            catch (const DatabaseException& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            catch (const std::exception& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            res.end();
        });
}
