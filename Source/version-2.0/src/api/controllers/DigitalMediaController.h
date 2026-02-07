#pragma once
#include <crow.h>
#include <memory>
#include "src/application/services/DigitalMediaService.h"
#include "src/api/middleware/JwtMiddleware.h"
#include "src/api/middleware/PermissionMiddleware.h"
#include "src/utils/JsonUtils.h"
#include "src/utils/Exceptions.h"

class DigitalMediaController {
public:
    explicit DigitalMediaController(std::shared_ptr<DigitalMediaService> service)
        : service_(std::move(service)) {}

    void registerRoutes(crow::App<JwtMiddleware, PermissionMiddleware>& app);

private:
    std::shared_ptr<DigitalMediaService> service_;
};
