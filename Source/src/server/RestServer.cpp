#include "httplib.h"
#include "Library.h"
#include "EnvLoader.h"
#include "MongoLogger.h"
#include "RestServer.h"
#include "Ultis.h"

#include <nlohmann/json.hpp>

void openRestServer() {
    httplib::Server svr;

    EnvLoader env;
    MongoLogger logger(env.getMongoUrl());
    Library library;

    // GET /medias
    svr.Get("/media", [&](const httplib::Request&, httplib::Response& res) {
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

    svr.Post("/media", [&](const httplib::Request& req, httplib::Response& res){
        try {
            auto j = nlohmann::json::parse(req.body);

            if (!j.contains("media_type") || !j.contains("id")) {
                res.status = 400;
                res.set_content(R"({"error": "Missing 'media_type' or 'id' field"})", "application/json");
                return;
            }

            if (library.findItemById(j["id"]) != nullptr){
                res.status = 400;
                res.set_content(R"({"error": "Duplicate id"})", "application/json");
                return;
            };

            std::string mediaType = j["media_type"];

            if (mediaType == "BOOK") {
                if (!j.contains("title") || !j.contains("author")) {
                    res.status = 400;
                    res.set_content(R"({"error": "Books require 'title' and 'author'"})", "application/json");
                    return;
                }
                if (!j["title"].is_string() || !j["author"].is_string()) {
                    res.status = 400;
                    res.set_content(R"({"error": "Invalid field types"})", "application/json");
                    return;
                }
                
                library.addBook(j["id"], j["title"], j["author"]);
                
                res.status = 201;
                res.set_content(R"({"SUCCESS": "Item added successfully"})", "application/json");
                
            } else if (mediaType == "MAGAZINE") {
                if (!j.contains("title") || !j.contains("issue_number")) {
                    res.status = 400;
                    res.set_content(R"({"error": "Magazines require 'title' and 'issue_number'"})", "application/json");
                    return;
                }
                if (!j["title"].is_string() || !j["issue_number"].is_number_integer()) {
                    res.status = 400;
                    res.set_content(R"({"error": "Invalid field types"})", "application/json");
                    return;
                }
                
                library.addMagazine(j["id"], j["title"], j["issue_number"]);
                
                res.status = 201;
                res.set_content(R"({"SUCCESS": "Item added successfully"})", "application/json");
                
            } else {
                res.status = 400;
                nlohmann::json error = {
                    {"error", "Invalid media_type"},
                    {"allowed_values", {"BOOK", "MAGAZINE"}}
                };
                res.set_content(error.dump(), "application/json");
            }
            
        } catch (nlohmann::json::parse_error& e) {
            res.status = 400;
            res.set_content(R"({"error": "Invalid JSON"})", "application/json");
        } catch (std::exception& e) {
            res.status = 500;
            res.set_content(R"({"error": "Internal server error"})", "application/json");
        }

    });

    // POST /borrow
    svr.Post("/borrow", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = nlohmann::json::parse(req.body);
        int userId = body["userId"];
        int mediaId = body["mediaId"];

        auto result = Ultis::tryExecute(logger, "Borrow Media", userId, [&](){ library.borrowItem(mediaId, userId); });

        res.status = result.httpCode;
        res.set_content(
            R"({"message": ")" + result.message + R"("})",
            "application/json"
        );
    });

    // POST /return
    svr.Post("/return", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = nlohmann::json::parse(req.body);
        int userId = body["userId"];
        int mediaId = body["mediaId"];

        auto result = Ultis::tryExecute(logger, "Return Media", userId, [&](){ library.returnItem(mediaId,userId); });
        
        res.status = result.httpCode;
        res.set_content(
            R"({"message": ")" + result.message + R"("})",
            "application/json"
        );
    });

    std::cout << "Server listening on http://localhost:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);
}
