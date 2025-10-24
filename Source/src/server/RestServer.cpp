#include "httplib.h"
#include "Library.h"
#include "EnvLoader.h"
#include "MongoLogger.h"
#include "RestServer.h"

#include <nlohmann/json.hpp>

void openRestServer() {
    httplib::Server svr;

    EnvLoader env;
    MongoLogger logger(env.getMongoUrl());
    Library library;

    // GET /medias
    svr.Get("/medias", [&](const httplib::Request&, httplib::Response& res) {
        auto medias = library.getAllMedias();
        nlohmann::json j = nlohmann::json::array();

        for (const auto& m : medias) {
            nlohmann::json entry;
            entry["id"] = m->getId();
            entry["title"] = m->getName();
            entry["available"] = m->getAvailability();

            // Detect the type at runtime
            if (auto book = std::dynamic_pointer_cast<Book>(m)) {
                entry["type"] = "BOOK";
                entry["author"] = book->getAuthor();
            } 
            else if (auto mag = std::dynamic_pointer_cast<Magazine>(m)) {
                entry["type"] = "MAGAZINE";
                entry["issue_number"] = mag->getIssueNumber();
            } 
            else {
                entry["type"] = "UNKNOWN";
            }

            j.push_back(entry);
        }
        res.set_content(j.dump(), "application/json");
    });

    // POST /borrow
    svr.Post("/borrow", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = nlohmann::json::parse(req.body);
        int userId = body["userId"];
        int mediaId = body["mediaId"];
        library.borrowItem(userId, mediaId);
        logger.logEvent("INFO", "User " + std::to_string(userId) + " borrowed media " + std::to_string(mediaId));
        res.set_content("{\"status\":\"borrowed\"}", "application/json");
    });

    // POST /return
    svr.Post("/return", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = nlohmann::json::parse(req.body);
        int userId = body["userId"];
        int mediaId = body["mediaId"];
        library.returnItem(userId, mediaId);
        logger.logEvent("INFO", "Returned media id " + std::to_string(mediaId) + " by user " + std::to_string(userId));
        res.set_content("{\"status\":\"returned\"}", "application/json");
    });

    std::cout << "Server listening on http://localhost:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);
}
