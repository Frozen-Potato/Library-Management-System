#include "SearchController.h"

void SearchController::registerRoutes(crow::App<JwtMiddleware, PermissionMiddleware>& app) {

    CROW_ROUTE(app, "/api/search").methods(crow::HTTPMethod::GET)(
        [this, &app](const crow::request& req, crow::response& res) {
            try {
                const auto& ctx = app.get_context<JwtMiddleware>(req);

                // Extract query string
                auto query = req.url_params.get("q");
                if (!query || std::string(query).empty()) {
                    res.code = 400;
                    res.write(makeJsonError("Missing query parameter 'q'"));
                    res.end();
                    return;
                }

                // Pagination
                int from = 0;
                int size = 10;

                if (req.url_params.get("from"))
                    from = std::max(0, std::atoi(req.url_params.get("from")));
                
                if (req.url_params.get("size"))
                    size = std::max(1, std::atoi(req.url_params.get("size")));

                auto results = library_->searchMedia(query);

                // Manual pagination
                int end = std::min((int)results.size(), from + size);
                std::vector<nlohmann::json> page;

                for (int i = from; i < end; ++i) {
                    page.push_back(results[i]);
                }

                nlohmann::json response {
                    {"total", results.size()},
                    {"from", from},
                    {"size", size},
                    {"results", page}
                };

                res.code = 200;
                res.write(response.dump());
                res.end();
            }
            catch (const DatabaseException& e) {
                res.code = 500;
                res.write(makeJsonError(e.what()));
                res.end();
            }
            catch (const std::exception& e) {
                res.code = 500;
                res.write(makeJsonError(std::string("Search failed: ") + e.what()));
                res.end();
            }
        }
    );
}
