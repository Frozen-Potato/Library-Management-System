#include "src/data/PostgresAdapter.h"
#include "src/domain/media/MediaCopy.h"
#include "src/utils/DateTimeUtils.h"
#include <stdexcept>

PostgresAdapter::PostgresAdapter(std::shared_ptr<pqxx::connection> conn)
    : conn_(std::move(conn)) {}

// ------------------------------------------------ MEDIA ------------------------------------------------
long PostgresAdapter::createMedia(int mediaTypeId, const std::string& title) {
    pqxx::work txn(*conn_);
    auto r = txn.exec_params(
        "INSERT INTO media (title, media_type_id, is_available) VALUES ($1, $2, TRUE) RETURNING id;",
        title, mediaTypeId
    );
    txn.commit();
    return r[0]["id"].as<long>();
}

void PostgresAdapter::attachBook(long mediaId, const std::string& author, const std::string& isbn) {
    pqxx::work txn(*conn_);
    txn.exec_params(
        "INSERT INTO book (media_id, author, isbn) VALUES ($1, $2, $3) "
        "ON CONFLICT (media_id) DO NOTHING;",
        mediaId, author, isbn
    );
    txn.commit();
}

void PostgresAdapter::attachMagazine(long mediaId, int issueNumber, const std::string& publisher) {
    pqxx::work txn(*conn_);
    txn.exec_params(
        "INSERT INTO magazine (media_id, issue_number, publisher) VALUES ($1, $2, $3) "
        "ON CONFLICT (media_id) DO NOTHING;",
        mediaId, issueNumber, publisher
    );
    txn.commit();
}

MediaCopy PostgresAdapter::createMediaCopy(long mediaId, const std::string& condition) {
    pqxx::work txn(*conn_);
    txn.exec_params("CALL create_media_copy($1, $2);", mediaId, condition);
    auto r = txn.exec_params(
        "SELECT copy_id, media_id, condition, is_available "
        "FROM media_copy WHERE media_id = $1 ORDER BY copy_id DESC LIMIT 1;",
        mediaId
    );
    txn.commit();

    if (r.empty()) throw std::runtime_error("Failed to create media copy.");

    MediaCopy c;
    c.copyId = r[0]["copy_id"].as<long>();
    c.mediaId = r[0]["media_id"].as<long>();
    c.condition = condition_from_string(r[0]["condition"].as<std::string>());
    c.isAvailable = r[0]["is_available"].as<bool>();
    return c;
}

MediaCopy PostgresAdapter::getCopy(long copyId) {
    pqxx::work txn(*conn_);
    auto r = txn.exec_params(
        "SELECT copy_id, media_id, condition, is_available "
        "FROM media_copy WHERE copy_id = $1;",
        copyId
    );
    txn.commit();
    if (r.empty()) throw std::runtime_error("Copy not found.");

    MediaCopy c;
    c.copyId = r[0]["copy_id"].as<long>();
    c.mediaId = r[0]["media_id"].as<long>();
    c.condition = condition_from_string(r[0]["condition"].as<std::string>());
    c.isAvailable = r[0]["is_available"].as<bool>();
    return c;
}

std::vector<MediaCopy> PostgresAdapter::listCopiesByMedia(long mediaId) {
    pqxx::work txn(*conn_);
    auto res = txn.exec_params(
        "SELECT copy_id, media_id, condition, is_available "
        "FROM media_copy WHERE media_id = $1 ORDER BY copy_id;",
        mediaId
    );
    txn.commit();

    std::vector<MediaCopy> out;
    out.reserve(res.size());
    for (const auto& row : res) {
        MediaCopy c;
        c.copyId = row["copy_id"].as<long>();
        c.mediaId = row["media_id"].as<long>();
        c.condition = condition_from_string(row["condition"].as<std::string>());
        c.isAvailable = row["is_available"].as<bool>();
        out.push_back(c);
    }
    return out;
}

std::vector<std::shared_ptr<Media>> PostgresAdapter::getAllMedia() {
    pqxx::work txn(*conn_);
    auto res = txn.exec(
        "SELECT id, title, is_available FROM media ORDER BY id;"
    );
    txn.commit();

    std::vector<std::shared_ptr<Media>> out;
    for (const auto& row : res) {
        out.push_back(std::make_shared<Media>(
            row["id"].as<long>(),
            row["title"].as<std::string>(),
            row["is_available"].as<bool>()
        ));
    }
    return out;
}

// ------------------------------------------------ BORROW ------------------------------------------------
void PostgresAdapter::addActiveBorrow(int userId, long copyId) {
    pqxx::work txn(*conn_);
    txn.exec_params("CALL add_active_borrow($1, $2);", userId, copyId);
    txn.commit();
}

void PostgresAdapter::markCopyReturned(long copyId) {
    pqxx::work txn(*conn_);
    txn.exec_params("CALL mark_copy_returned($1);", copyId);
    txn.commit();
}

std::optional<BorrowRecord> PostgresAdapter::findActiveBorrow(int userId, long copyId) {
    pqxx::work txn(*conn_);
    auto r = txn.exec_params(
        "SELECT borrow_id, user_id, copy_id, borrow_date "
        "FROM active_borrow WHERE user_id = $1 AND copy_id = $2;",
        userId, copyId
    );
    txn.commit();
    if (r.empty()) return std::nullopt;

    return BorrowRecord(
        r[0]["borrow_id"].as<int>(),
        r[0]["user_id"].as<int>(),
        r[0]["copy_id"].as<long>(),
        parseTimestamp(r[0]["borrow_date"].as<std::string>()),
        std::nullopt
    );
}

// ------------------------------------------------ USER ------------------------------------------------
int PostgresAdapter::insertUser(const std::string& name, const std::string& email,
                                const std::string& hashedPassword, const std::string& role) {
    pqxx::work txn(*conn_);
    auto r = txn.exec_params(
        "INSERT INTO users (name, email, password_hash) VALUES ($1, $2, $3) RETURNING id;",
        name, email, hashedPassword
    );
    int id = r[0]["id"].as<int>();
    txn.exec_params("INSERT INTO user_roles (user_id, role_id) "
                    "SELECT $1, id FROM roles WHERE name = $2;", id, role);
    txn.commit();
    return id;
}

void PostgresAdapter::assignRole(int userId, const std::string& role) {
    pqxx::work txn(*conn_);
    txn.exec_params(
        "INSERT INTO user_roles (user_id, role_id) "
        "SELECT $1, id FROM roles WHERE name = $2 "
        "ON CONFLICT (user_id, role_id) DO NOTHING;",
        userId, role
    );
    txn.commit();
}

std::vector<std::shared_ptr<User>> PostgresAdapter::getAllUsers() {
    pqxx::work txn(*conn_);
    auto res = txn.exec(
        "SELECT id, name, email FROM users ORDER BY id;"
    );
    txn.commit();

    std::vector<std::shared_ptr<User>> out;
    for (const auto& row : res) {
        out.push_back(std::make_shared<User>(
            row["id"].as<int>(),
            row["name"].as<std::string>(),
            row["email"].as<std::string>()
        ));
    }
    return out;
}

std::optional<UserRow> PostgresAdapter::getUserByName(const std::string& username) {
    pqxx::work txn(*conn_);
    auto r = txn.exec_params(
        "SELECT u.id, u.name, u.email, u.password_hash, r.name as role "
        "FROM users u "
        "LEFT JOIN user_roles ur ON u.id = ur.user_id "
        "LEFT JOIN roles r ON ur.role_id = r.id "
        "WHERE u.name = $1 LIMIT 1;",
        username
    );
    txn.commit();

    if (r.empty()) return std::nullopt;

    return UserRow{
        r[0]["id"].as<int>(),
        r[0]["name"].as<std::string>(),
        r[0]["email"].as<std::string>(),
        r[0]["password_hash"].as<std::string>(),
        r[0]["role"].as<std::string>()
    };
}
