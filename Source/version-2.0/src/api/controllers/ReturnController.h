#pragma once
#include <crow.h>
#include <memory>
#include "src/application/services/LibraryService.h"
#include "src/api/middleware/JwtMiddleware.h"

class ReturnController {
public:
    explicit ReturnController(std::shared_ptr<LibraryService> service);
    void registerRoutes(crow::App<JwtMiddleware>& app);

private:
    std::shared_ptr<LibraryService> service_;
};
