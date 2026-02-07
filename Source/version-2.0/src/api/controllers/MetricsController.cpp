#include "MetricsController.h"

void MetricsController::registerRoutes(crow::App<JwtMiddleware, PermissionMiddleware>& app) {

    // GET /metrics - Prometheus scrape endpoint (no auth required)
    CROW_ROUTE(app, "/metrics").methods(crow::HTTPMethod::GET)(
        [](const crow::request&) {
            auto body = MetricsRegistry::instance().serialize();
            auto resp = crow::response(200, body);
            resp.set_header("Content-Type", "text/plain; version=0.0.4; charset=utf-8");
            return resp;
        });

    // GET /health - Health check
    CROW_ROUTE(app, "/health").methods(crow::HTTPMethod::GET)(
        [](const crow::request&) {
            nlohmann::json health = {
                {"status", "healthy"},
                {"service", "library-management-system"},
                {"version", "2.0"}
            };
            return crow::response(200, health.dump());
        });

    // GET /ready - Readiness check
    CROW_ROUTE(app, "/ready").methods(crow::HTTPMethod::GET)(
        [](const crow::request&) {
            nlohmann::json ready = {
                {"status", "ready"}
            };
            return crow::response(200, ready.dump());
        });
}
