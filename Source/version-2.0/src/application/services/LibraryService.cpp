#include "src/application/services/LibraryService.h"
#include "src/utils/Exceptions.h"
#include "src/utils/DateTimeUtils.h"
#include <iostream>

LibraryService::LibraryService(std::shared_ptr<PostgresAdapter> db,
                               std::shared_ptr<QueueService> queue)
    : db_(std::move(db)), queue_(std::move(queue)) {}

// --- Media creation ---

long LibraryService::createBook(int mediaTypeId, const std::string& title,
                                const std::string& author, const std::string& isbn) {
    if (title.empty() || author.empty())
        throw ValidationException("Title and Author cannot be empty");

    std::lock_guard<std::mutex> lock(mtx_);
    try {
        long mediaId = db_->createMedia(mediaTypeId, title);
        db_->attachBook(mediaId, author, isbn);
        enqueueLog("CREATE_BOOK", 0, mediaId);
        return mediaId;
    } catch (const std::exception& e) {
        throw DatabaseException("Failed to create book: " + std::string(e.what()));
    }
}

long LibraryService::createMagazine(int mediaTypeId, const std::string& title,
                                    int issueNumber, const std::string& publisher) {
    if (title.empty() || publisher.empty())
        throw ValidationException("Title and Publisher cannot be empty");

    std::lock_guard<std::mutex> lock(mtx_);
    try {
        long mediaId = db_->createMedia(mediaTypeId, title);
        db_->attachMagazine(mediaId, issueNumber, publisher);
        enqueueLog("CREATE_MAGAZINE", 0, mediaId);
        return mediaId;
    } catch (const std::exception& e) {
        throw DatabaseException("Failed to create magazine: " + std::string(e.what()));
    }
}

long LibraryService::createCopy(long mediaId, const std::string& condition) {
    if (mediaId <= 0)
        throw ValidationException("Invalid media ID");

    std::lock_guard<std::mutex> lock(mtx_);
    try {
        auto copy = db_->createMediaCopy(mediaId, condition);
        enqueueLog("CREATE_COPY", 0, copy.copyId);
        return copy.copyId;
    } catch (const std::exception& e) {
        throw DatabaseException("Failed to create copy: " + std::string(e.what()));
    }
}

// --- Borrow / Return ---

void LibraryService::borrowCopy(int userId, long copyId) {
    if (userId <= 0 || copyId <= 0)
        throw ValidationException("Invalid user or copy ID");

    std::lock_guard<std::mutex> lock(mtx_);
    try {
        auto copy = db_->getCopy(copyId);
        if (!copy.isAvailable)
            throw ValidationException("Copy not available for borrowing");

        db_->addActiveBorrow(userId, copyId);
        enqueueLog("BORROW_COPY", userId, copyId);
    } catch (const ValidationException&) {
        throw;
    } catch (const std::exception& e) {
        throw DatabaseException("Borrow operation failed: " + std::string(e.what()));
    }
}

void LibraryService::returnCopy(int userId, long copyId) {
    if (userId <= 0 || copyId <= 0)
        throw ValidationException("Invalid user or copy ID");

    std::lock_guard<std::mutex> lock(mtx_);
    try {
        auto borrow = db_->findActiveBorrow(userId, copyId);
        if (!borrow.has_value())
            throw ValidationException("Copy not borrowed by this user");

        db_->markCopyReturned(copyId);
        enqueueLog("RETURN_COPY", userId, copyId);
    } catch (const ValidationException&) {
        throw;
    } catch (const std::exception& e) {
        throw DatabaseException("Return operation failed: " + std::string(e.what()));
    }
}

// --- Queries ---

std::vector<std::shared_ptr<Media>> LibraryService::getAllMedia() {
    try {
        return db_->getAllMedia();
    } catch (const std::exception& e) {
        throw DatabaseException("Failed to fetch media: " + std::string(e.what()));
    }
}

std::vector<MediaCopy> LibraryService::listCopiesByMedia(long mediaId) {
    if (mediaId <= 0)
        throw ValidationException("Invalid media ID");

    try {
        return db_->listCopiesByMedia(mediaId);
    } catch (const std::exception& e) {
        throw DatabaseException("Failed to fetch copies: " + std::string(e.what()));
    }
}

// --- Private helper ---

void LibraryService::enqueueLog(const std::string& action, int userId, long entityId) {
    try {
        nlohmann::json log = {
            {"timestamp", nowToString()},
            {"action", action},
            {"user_id", userId},
            {"entity_id", entityId}
        };
        queue_->enqueue("AUDIT_LOG", log);
    } catch (const std::exception& e) {
        std::cerr << "[LibraryService] Failed to enqueue log: " << e.what() << std::endl;
    }
}