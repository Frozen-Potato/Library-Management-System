#pragma once
#include <crow.h>
#include "src/infrastructure/metrics/MetricsRegistry.h"
#include "src/api/middleware/JwtMiddleware.h"
#include "src/api/middleware/PermissionMiddleware.h"

class MetricsController {
public:
    void registerRoutes(crow::App<JwtMiddleware, PermissionMiddleware>& app);
};
