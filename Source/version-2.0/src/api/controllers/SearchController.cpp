#include "SearchController.h"

void SearchController::registerRoutes(crow::App<JwtMiddleware, PermissionMiddleware>& app) {

    // GET /api/search - Full-text search with fuzzy matching
    CROW_ROUTE(app, "/api/search").methods(crow::HTTPMethod::GET)(
        [this, &app](const crow::request& req, crow::response& res) {
            try {
                const auto& ctx = app.get_context<JwtMiddleware>(req);

                auto query = req.url_params.get("q");
                if (!query || std::string(query).empty()) {
                    res.code = 400;
                    res.write(makeJsonError("Missing query parameter 'q'"));
                    res.end();
                    return;
                }

                int from = 0;
                int size = 10;
                if (req.url_params.get("from"))
                    from = std::max(0, std::atoi(req.url_params.get("from")));
                if (req.url_params.get("size"))
                    size = std::max(1, std::atoi(req.url_params.get("size")));

                std::vector<nlohmann::json> results;
                if (search_) {
                    // Use fuzzy search with configurable fuzziness
                    std::string fuzziness = "AUTO";
                    if (req.url_params.get("fuzziness"))
                        fuzziness = req.url_params.get("fuzziness");
                    results = search_->fuzzySearch(query, fuzziness, from, size);
                } else {
                    // Fallback to basic search via LibraryService
                    auto all = library_->searchMedia(query);
                    int end = std::min(static_cast<int>(all.size()), from + size);
                    for (int i = from; i < end; ++i)
                        results.push_back(all[i]);
                }

                nlohmann::json response = {
                    {"total", results.size()},
                    {"from", from},
                    {"size", size},
                    {"results", results}
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

    // GET /api/search/suggest - Auto-suggest / completion
    CROW_ROUTE(app, "/api/search/suggest").methods(crow::HTTPMethod::GET)(
        [this](const crow::request& req, crow::response& res) {
            try {
                auto prefix = req.url_params.get("q");
                if (!prefix || std::string(prefix).empty()) {
                    res.code = 400;
                    res.write(makeJsonError("Missing query parameter 'q'"));
                    res.end();
                    return;
                }

                int maxResults = 5;
                if (req.url_params.get("limit"))
                    maxResults = std::max(1, std::atoi(req.url_params.get("limit")));

                std::vector<std::string> suggestions;
                if (search_) {
                    suggestions = search_->autoSuggest(prefix, maxResults);
                }

                nlohmann::json response = {
                    {"query", std::string(prefix)},
                    {"suggestions", suggestions}
                };

                res.code = 200;
                res.write(response.dump());
            }
            catch (const std::exception& e) {
                res.code = 500;
                res.write(makeJsonError(std::string("Suggest failed: ") + e.what()));
            }
            res.end();
        }
    );
}
