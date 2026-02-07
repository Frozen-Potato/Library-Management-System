#pragma once
#include <memory>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "src/data/PostgresAdapter.h"
#include "src/infrastructure/search/OpenSearchClient.h"
#include "src/infrastructure/messaging/KafkaProducer.h"

struct ImportResult {
    int totalRecords;
    int successCount;
    int failureCount;
    std::vector<std::string> errors;
};

class BatchImportService {
public:
    BatchImportService(std::shared_ptr<PostgresAdapter> db,
                       std::shared_ptr<OpenSearchClient> search,
                       std::shared_ptr<KafkaProducer> events);

    // Import from JSON array
    ImportResult importFromJson(const nlohmann::json& records);

    // Import from CSV string
    ImportResult importFromCsv(const std::string& csvData);

private:
    ImportResult processRecords(const std::vector<nlohmann::json>& records);
    std::vector<nlohmann::json> parseCsv(const std::string& csvData);

    std::shared_ptr<PostgresAdapter> db_;
    std::shared_ptr<OpenSearchClient> search_;
    std::shared_ptr<KafkaProducer> events_;
};
