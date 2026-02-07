#pragma once
#include <crow.h>
#include <memory>
#include "src/application/services/LibraryService.h"
#include "src/api/middleware/JwtMiddleware.h"
#include "src/api/middleware/PermissionMiddleware.h"
#include "src/utils/JsonUtils.h"

class SearchController {
public:
    SearchController(std::shared_ptr<LibraryService> library)
        : library_(std::move(library)) {}

    void registerRoutes(crow::App<JwtMiddleware, PermissionMiddleware>& app);

private:
    std::shared_ptr<LibraryService> library_;
};
