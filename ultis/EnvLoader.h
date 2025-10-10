#ifndef ENV_LOADER_H
#define ENV_LOADER_H

#include <fstream>
#include <string>
#include <stdexcept>

class EnvLoader {
private:
    std::string dbUrl;
    std::string mongoUrl;

public:
    explicit EnvLoader(const std::string& path = ".env") {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open .env file");
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.rfind("DATABASE_URL=", 0) == 0) {
                dbUrl = line.substr(13); // "DATABASE_URL="
            } else if (line.rfind("MONGO_URL=", 0) == 0) {
                mongoUrl = line.substr(10); // "MONGO_URL="
            }
        }

        if (dbUrl.empty()) {
            throw std::runtime_error("DATABASE_URL not found in .env");
        }
        if (mongoUrl.empty()) {
            throw std::runtime_error("MONGO_URL not found in .env");
        }
    }

    std::string getDatabaseUrl() const { return dbUrl; }
    std::string getMongoUrl() const { return mongoUrl; }
};

#endif
