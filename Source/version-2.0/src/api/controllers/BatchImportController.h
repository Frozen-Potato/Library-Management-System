#pragma once
#include <crow.h>
#include <memory>
#include "src/application/services/BatchImportService.h"
#include "src/api/middleware/JwtMiddleware.h"
#include "src/api/middleware/PermissionMiddleware.h"
#include "src/utils/JsonUtils.h"
#include "src/utils/Exceptions.h"

class BatchImportController {
public:
    explicit BatchImportController(std::shared_ptr<BatchImportService> service)
        : service_(std::move(service)) {}

    void registerRoutes(crow::App<JwtMiddleware, PermissionMiddleware>& app);

private:
    std::shared_ptr<BatchImportService> service_;
};
