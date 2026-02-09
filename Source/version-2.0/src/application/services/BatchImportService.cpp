#include "BatchImportService.h"
#include "src/utils/Exceptions.h"
#include "src/utils/StringUtils.h"
#include "src/utils/DateTimeUtils.h"
#include <sstream>
#include <iostream>

BatchImportService::BatchImportService(std::shared_ptr<PostgresAdapter> db,
                                       std::shared_ptr<OpenSearchClient> search,
                                       std::shared_ptr<KafkaProducer> events)
    : db_(std::move(db)), search_(std::move(search)), events_(std::move(events)) {}

std::vector<nlohmann::json> BatchImportService::parseCsv(const std::string& csvData) {
    std::vector<nlohmann::json> records;
    std::istringstream stream(csvData);
    std::string line;

    // Parse header row
    std::vector<std::string> headers;
    if (std::getline(stream, line)) {
        headers = split(line, ',');
        for (auto& h : headers) h = trim(h);
    }

    if (headers.empty())
        throw ValidationException("CSV has no header row");

    // Parse data rows
    while (std::getline(stream, line)) {
        if (trim(line).empty()) continue;

        auto values = split(line, ',');
        nlohmann::json record;
        for (size_t i = 0; i < headers.size() && i < values.size(); ++i) {
            std::string val = trim(values[i]);
            // Remove surrounding quotes if present
            if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
                val = val.substr(1, val.size() - 2);
            }
            record[headers[i]] = val;
        }
        records.push_back(record);
    }

    return records;
}

ImportResult BatchImportService::importFromCsv(const std::string& csvData) {
    if (csvData.empty())
        throw ValidationException("CSV data is empty");

    auto records = parseCsv(csvData);
    return processRecords(records);
}

ImportResult BatchImportService::importFromJson(const nlohmann::json& records) {
    if (!records.is_array())
        throw ValidationException("Expected JSON array of records");

    std::vector<nlohmann::json> vec;
    for (auto& r : records) {
        vec.push_back(r);
    }
    return processRecords(vec);
}

ImportResult BatchImportService::processRecords(const std::vector<nlohmann::json>& records) {
    ImportResult result;
    result.totalRecords = static_cast<int>(records.size());
    result.successCount = 0;
    result.failureCount = 0;

    std::vector<nlohmann::json> searchDocs;

    for (size_t i = 0; i < records.size(); ++i) {
        const auto& rec = records[i];
        try {
            std::string type = rec.value("type", "Book");
            std::string title = rec.value("title", "");

            if (title.empty()) {
                result.errors.push_back("Row " + std::to_string(i + 1) + ": missing title");
                result.failureCount++;
                continue;
            }

            // Determine media_type_id
            int mediaTypeId = 1; // Default: Book
            if (type == "Magazine") mediaTypeId = 2;
            else if (type == "DVD") mediaTypeId = 3;
            else if (type == "AudioBook") mediaTypeId = 4;
            else if (type == "DigitalMedia") mediaTypeId = 5;

            long mediaId = db_->createMedia(mediaTypeId, title);

            if (type == "Book") {
                std::string author = rec.value("author", "Unknown");
                std::string isbn = rec.value("isbn", "");
                db_->attachBook(mediaId, author, isbn);

                searchDocs.push_back({
                    {"id", mediaId}, {"title", title},
                    {"author", author}, {"category", "Book"},
                    {"suggest", nlohmann::json::array({title, author})}
                });
            }
            else if (type == "Magazine") {
                int issue = 0;
                if (rec.contains("issue_number")) {
                    auto& iv = rec["issue_number"];
                    issue = iv.is_string() ? std::stoi(iv.get<std::string>()) : iv.get<int>();
                }
                std::string publisher = rec.value("publisher", "Unknown");
                db_->attachMagazine(mediaId, issue, publisher);

                searchDocs.push_back({
                    {"id", mediaId}, {"title", title},
                    {"author", publisher}, {"category", "Magazine"},
                    {"suggest", nlohmann::json::array({title, publisher})}
                });
            }

            result.successCount++;

        } catch (const std::exception& e) {
            result.errors.push_back("Row " + std::to_string(i + 1) + ": " + e.what());
            result.failureCount++;
        }
    }

    // Bulk index to OpenSearch
    if (!searchDocs.empty()) {
        try {
            search_->bulkIndex(searchDocs);
        } catch (const std::exception& e) {
            std::cerr << "[BatchImport] OpenSearch bulk index failed: " << e.what() << std::endl;
        }
    }

    // Publish import event
    events_->produceJson("media.events", "batch.import", {
        {"event", "BATCH_IMPORT_COMPLETED"},
        {"total", result.totalRecords},
        {"success", result.successCount},
        {"failures", result.failureCount},
        {"timestamp", nowToString()}
    });

    return result;
}
