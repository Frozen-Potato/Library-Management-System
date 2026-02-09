#include "BatchImportController.h"

using json = nlohmann::json;

void BatchImportController::registerRoutes(crow::App<JwtMiddleware, PermissionMiddleware>& app) {

    // POST /api/import/json - Bulk import from JSON array
    CROW_ROUTE(app, "/api/import/json").methods(crow::HTTPMethod::POST)(
        [this, &app](const crow::request& req, crow::response& res) {
            try {
                const auto& ctx = app.get_context<JwtMiddleware>(req);
                if (!ctx.valid) {
                    res.code = 401;
                    res.write(makeJsonError("Unauthorized"));
                    res.end();
                    return;
                }

                auto body = json::parse(req.body, nullptr, false);
                if (body.is_discarded()) {
                    res.code = 400;
                    res.write(makeJsonError("Invalid JSON"));
                    res.end();
                    return;
                }

                // Accept either {"records": [...]} or a raw array [...]
                json records;
                if (body.is_array()) {
                    records = body;
                } else if (body.contains("records") && body["records"].is_array()) {
                    records = body["records"];
                } else {
                    res.code = 400;
                    res.write(makeJsonError("Expected JSON array or object with 'records' array"));
                    res.end();
                    return;
                }

                auto result = service_->importFromJson(records);

                json resp = {
                    {"status", result.failureCount == 0 ? "success" : "partial"},
                    {"total", result.totalRecords},
                    {"imported", result.successCount},
                    {"failed", result.failureCount},
                    {"errors", result.errors}
                };

                res.code = (result.failureCount == 0) ? 200 : 207;
                res.write(resp.dump());
            }
            catch (const ValidationException& e) { res.code = 400; res.write(makeJsonError(e.what())); }
            catch (const std::exception& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            res.end();
        });

    // POST /api/import/csv - Bulk import from CSV
    CROW_ROUTE(app, "/api/import/csv").methods(crow::HTTPMethod::POST)(
        [this, &app](const crow::request& req, crow::response& res) {
            try {
                const auto& ctx = app.get_context<JwtMiddleware>(req);
                if (!ctx.valid) {
                    res.code = 401;
                    res.write(makeJsonError("Unauthorized"));
                    res.end();
                    return;
                }

                if (req.body.empty()) {
                    res.code = 400;
                    res.write(makeJsonError("Empty CSV body"));
                    res.end();
                    return;
                }

                auto result = service_->importFromCsv(req.body);

                json resp = {
                    {"status", result.failureCount == 0 ? "success" : "partial"},
                    {"total", result.totalRecords},
                    {"imported", result.successCount},
                    {"failed", result.failureCount},
                    {"errors", result.errors}
                };

                res.code = (result.failureCount == 0) ? 200 : 207;
                res.write(resp.dump());
            }
            catch (const ValidationException& e) { res.code = 400; res.write(makeJsonError(e.what())); }
            catch (const std::exception& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            res.end();
        });
}
