#pragma once
#include <crow.h>
#include <memory>
#include "src/application/services/LibraryService.h"
#include "src/infrastructure/search/OpenSearchClient.h"
#include "src/api/middleware/JwtMiddleware.h"
#include "src/api/middleware/PermissionMiddleware.h"
#include "src/utils/JsonUtils.h"

class SearchController {
public:
    SearchController(std::shared_ptr<LibraryService> library,
                     std::shared_ptr<OpenSearchClient> search = nullptr)
        : library_(std::move(library)), search_(std::move(search)) {}

    void registerRoutes(crow::App<JwtMiddleware, PermissionMiddleware>& app);

private:
    std::shared_ptr<LibraryService> library_;
    std::shared_ptr<OpenSearchClient> search_;
};
