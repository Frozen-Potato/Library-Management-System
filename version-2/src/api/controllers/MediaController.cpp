#include "src/api/controllers/MediaController.h"
#include "src/utils/JsonUtils.h"
#include "src/utils/Exceptions.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

MediaController::MediaController(std::shared_ptr<LibraryService> libraryService)
    : library_(std::move(libraryService)) {}

void MediaController::registerRoutes(crow::App<JwtMiddleware>& app) {
    // GET /api/media
    CROW_ROUTE(app, "/api/media")
        .methods(crow::HTTPMethod::GET)
        ([this](const crow::request& req) {
            try {
                auto list = library_->getAllMedia();
                json j = json::array();
                for (auto& m : list) {
                    j.push_back({{"id", m->getId()}, {"title", m->getTitle()}});
                }
                return crow::response(200, j.dump());
            }
            catch (const std::exception& e) {
                return crow::response(500, makeJsonError(e.what()));
            }
        });

    // POST /api/media/book
    CROW_ROUTE(app, "/api/media/book").methods(crow::HTTPMethod::POST)(
        [this](const crow::request& req) {
            try {
                auto body = parseJsonSafe(req.body);

                if (!body.contains("title") || !body.contains("author"))
                    throw ValidationException("Missing required fields: title or author");

                long id = library_->createBook(
                    body.value("media_type_id", 1),
                    body["title"],
                    body["author"],
                    body.value("isbn", "")
                );

                return crow::response(201, makeJsonSuccess("Book created with ID " + std::to_string(id)));
            }
            catch (const ValidationException& e) { return crow::response(400, makeJsonError(e.what())); }
            catch (const DatabaseException& e) { return crow::response(500, makeJsonError(e.what())); }
            catch (const std::exception& e) { return crow::response(500, makeJsonError(e.what())); }
        });
}
