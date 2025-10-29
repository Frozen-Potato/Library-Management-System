#pragma once
#include <crow.h>
#include <memory>
#include "src/application/services/LibraryService.h"
#include "src/api/middleware/JwtMiddleware.h"

class MediaController {
public:
    explicit MediaController(std::shared_ptr<LibraryService> libraryService);
    void registerRoutes(crow::App<JwtMiddleware>& app);

private:
    std::shared_ptr<LibraryService> library_;
};
