#include "DigitalMediaController.h"

using json = nlohmann::json;

void DigitalMediaController::registerRoutes(crow::App<JwtMiddleware, PermissionMiddleware>& app) {

    // POST /api/digital-media/upload - Upload digital media
    CROW_ROUTE(app, "/api/digital-media/upload").methods(crow::HTTPMethod::POST)(
        [this, &app](const crow::request& req, crow::response& res) {
            try {
                const auto& ctx = app.get_context<JwtMiddleware>(req);
                if (!ctx.valid) {
                    res.code = 401;
                    res.write(makeJsonError("Unauthorized"));
                    res.end();
                    return;
                }

                auto body = parseJsonSafe(req.body);
                if (!body.contains("title") || !body.contains("mime_type") || !body.contains("file_data")) {
                    res.code = 400;
                    res.write(makeJsonError("Missing required fields: title, mime_type, file_data"));
                    res.end();
                    return;
                }

                auto result = service_->uploadMedia(
                    body["title"],
                    body["mime_type"],
                    body["file_data"],
                    body.value("drm_protected", false)
                );

                res.code = 201;
                res.write(result.dump());
            }
            catch (const ValidationException& e) { res.code = 400; res.write(makeJsonError(e.what())); }
            catch (const DatabaseException& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            catch (const std::exception& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            res.end();
        });

    // GET /api/digital-media/<int>/download-url - Get presigned download URL
    CROW_ROUTE(app, "/api/digital-media/<int>/download-url").methods(crow::HTTPMethod::GET)(
        [this, &app](const crow::request& req, crow::response& res, int mediaId) {
            try {
                const auto& ctx = app.get_context<JwtMiddleware>(req);
                if (!ctx.valid) {
                    res.code = 401;
                    res.write(makeJsonError("Unauthorized"));
                    res.end();
                    return;
                }

                int expiry = 3600;
                if (req.url_params.get("expiry"))
                    expiry = std::atoi(req.url_params.get("expiry"));

                std::string url = service_->getDownloadUrl(mediaId, expiry);
                json resp = {{"download_url", url}, {"expires_in", expiry}};
                res.code = 200;
                res.write(resp.dump());
            }
            catch (const NotFoundException& e) { res.code = 404; res.write(makeJsonError(e.what())); }
            catch (const ValidationException& e) { res.code = 400; res.write(makeJsonError(e.what())); }
            catch (const std::exception& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            res.end();
        });

    // GET /api/digital-media/<int>/upload-url - Get presigned upload URL
    CROW_ROUTE(app, "/api/digital-media/<int>/upload-url").methods(crow::HTTPMethod::GET)(
        [this, &app](const crow::request& req, crow::response& res, int mediaId) {
            try {
                const auto& ctx = app.get_context<JwtMiddleware>(req);
                if (!ctx.valid) {
                    res.code = 401;
                    res.write(makeJsonError("Unauthorized"));
                    res.end();
                    return;
                }

                std::string mimeType = "application/octet-stream";
                if (req.url_params.get("mime_type"))
                    mimeType = req.url_params.get("mime_type");

                std::string url = service_->getUploadUrl(mediaId, mimeType);
                json resp = {{"upload_url", url}, {"media_id", mediaId}};
                res.code = 200;
                res.write(resp.dump());
            }
            catch (const NotFoundException& e) { res.code = 404; res.write(makeJsonError(e.what())); }
            catch (const std::exception& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            res.end();
        });

    // POST /api/digital-media/<int>/version - Create new version
    CROW_ROUTE(app, "/api/digital-media/<int>/version").methods(crow::HTTPMethod::POST)(
        [this, &app](const crow::request& req, crow::response& res, int mediaId) {
            try {
                const auto& ctx = app.get_context<JwtMiddleware>(req);
                if (!ctx.valid) {
                    res.code = 401;
                    res.write(makeJsonError("Unauthorized"));
                    res.end();
                    return;
                }

                auto body = parseJsonSafe(req.body);
                if (!body.contains("file_data")) {
                    res.code = 400;
                    res.write(makeJsonError("Missing file_data"));
                    res.end();
                    return;
                }

                auto result = service_->createVersion(mediaId, body["file_data"]);
                res.code = 201;
                res.write(result.dump());
            }
            catch (const NotFoundException& e) { res.code = 404; res.write(makeJsonError(e.what())); }
            catch (const ValidationException& e) { res.code = 400; res.write(makeJsonError(e.what())); }
            catch (const std::exception& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            res.end();
        });

    // GET /api/digital-media/<int>/versions - List versions
    CROW_ROUTE(app, "/api/digital-media/<int>/versions").methods(crow::HTTPMethod::GET)(
        [this, &app](const crow::request& req, crow::response& res, int mediaId) {
            try {
                const auto& ctx = app.get_context<JwtMiddleware>(req);
                if (!ctx.valid) {
                    res.code = 401;
                    res.write(makeJsonError("Unauthorized"));
                    res.end();
                    return;
                }

                auto versions = service_->listVersions(mediaId);
                json resp = {{"media_id", mediaId}, {"versions", versions}};
                res.code = 200;
                res.write(resp.dump());
            }
            catch (const std::exception& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            res.end();
        });

    // GET /api/digital-media/<int> - Get metadata
    CROW_ROUTE(app, "/api/digital-media/<int>").methods(crow::HTTPMethod::GET)(
        [this, &app](const crow::request& req, crow::response& res, int mediaId) {
            try {
                const auto& ctx = app.get_context<JwtMiddleware>(req);
                if (!ctx.valid) {
                    res.code = 401;
                    res.write(makeJsonError("Unauthorized"));
                    res.end();
                    return;
                }

                auto meta = service_->getMetadata(mediaId);
                if (!meta.has_value()) {
                    res.code = 404;
                    res.write(makeJsonError("Digital media not found"));
                    res.end();
                    return;
                }

                res.code = 200;
                res.write(meta->dump());
            }
            catch (const std::exception& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            res.end();
        });

    // DELETE /api/digital-media/<int> - Delete digital media
    CROW_ROUTE(app, "/api/digital-media/<int>").methods(crow::HTTPMethod::DELETE)(
        [this, &app](const crow::request& req, crow::response& res, int mediaId) {
            try {
                const auto& ctx = app.get_context<JwtMiddleware>(req);
                if (!ctx.valid) {
                    res.code = 401;
                    res.write(makeJsonError("Unauthorized"));
                    res.end();
                    return;
                }

                service_->deleteMedia(mediaId);
                res.code = 200;
                res.write(makeJsonSuccess("Digital media deleted"));
            }
            catch (const std::exception& e) { res.code = 500; res.write(makeJsonError(e.what())); }
            res.end();
        });
}
