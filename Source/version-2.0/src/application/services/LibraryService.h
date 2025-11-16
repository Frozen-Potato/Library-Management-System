#pragma once
#include <memory>
#include <mutex>
#include <optional>
#include <vector>
#include <string>

#include "src/domain/media/Media.h"
#include "src/domain/media/MediaCopy.h"
#include "src/domain/borrow/BorrowRecord.h"
#include "src/data/PostgresAdapter.h"
#include "src/application/services/QueueService.h"

// Application layer: core library workflow logic
class LibraryService {
public:
    LibraryService(std::shared_ptr<PostgresAdapter> db,
                   std::shared_ptr<QueueService> queue);

    // --- Media creation ---
    long createBook(int mediaTypeId, const std::string& title,
                    const std::string& author, const std::string& isbn);
    long createMagazine(int mediaTypeId, const std::string& title,
                        int issueNumber, const std::string& publisher);
    long createCopy(long mediaId, const std::string& condition = "GOOD");

    // --- Borrow / Return ---
    void borrowCopy(int userId, long copyId);
    void returnCopy(int userId, long copyId);

    // --- Queries ---
    std::vector<std::shared_ptr<Media>> getAllMedia();
    std::vector<MediaCopy> listCopiesByMedia(long mediaId);
    std::vector<nlohmann::json> searchMedia(const std::string& query);

private:
    void enqueueLog(const std::string& action, int userId, long entityId);

    std::shared_ptr<PostgresAdapter> db_;
    std::shared_ptr<QueueService> queue_;
    std::shared_ptr<OpenSearchClient> search_;
    std::mutex mtx_;
};
