#include <stdexcept>
#include "src/data/PostgresAdapter.h"

PostgresAdapter::PostgresAdapter(std::shared_ptr<pqxx::connection> conn)
    : conn_(std::move(conn)) {}

//  MEDIA 
long PostgresAdapter::createMedia(int mediaTypeId, const std::string& title) {
    pqxx::work txn(*conn_);
    auto r = txn.exec(
        "INSERT INTO media (title, media_type_id, is_available) VALUES ($1, $2, TRUE) RETURNING id;",
        pqxx::params{title, mediaTypeId}
    );
    txn.commit();
    return r[0]["id"].as<long>();
}

void PostgresAdapter::attachBook(long mediaId, const std::string& author, const std::string& isbn) {
    pqxx::work txn(*conn_);
    txn.exec(
        "INSERT INTO book (media_id, author, isbn) VALUES ($1, $2, $3) "
        "ON CONFLICT (media_id) DO NOTHING;",
        pqxx::params{mediaId, author, isbn}
    );
    txn.commit();
}

void PostgresAdapter::attachMagazine(long mediaId, int issueNumber, const std::string& publisher) {
    pqxx::work txn(*conn_);
    txn.exec(
        "INSERT INTO magazine (media_id, issue_number, publisher) VALUES ($1, $2, $3) "
        "ON CONFLICT (media_id) DO NOTHING;",
        pqxx::params{mediaId, issueNumber, publisher}
    );
    txn.commit();
}

MediaCopy PostgresAdapter::createMediaCopy(long mediaId, const std::string& condition) {
    pqxx::work txn(*conn_);
    txn.exec("CALL create_media_copy($1, $2);", pqxx::params{mediaId, condition});
    auto r = txn.exec(
        "SELECT copy_id, media_id, condition, is_available "
        "FROM media_copy WHERE media_id = $1 ORDER BY copy_id DESC LIMIT 1;",
        pqxx::params{mediaId}
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
    auto r = txn.exec(
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
    auto res = txn.exec(
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

    // Join with type-specific tables to get all details
    auto res = txn.exec(
        "SELECT "
        "    m.id, "
        "    m.title, "
        "    m.is_available, "
        "    mt.name AS type, "
        "    b.author, "
        "    b.isbn, "
        "    mg.issue_number, "
        "    mg.publisher, "
        "    d.director, "
        "    a.narrator "
        "FROM media m "
        "LEFT JOIN media_type mt ON m.media_type_id = mt.id "
        "LEFT JOIN book b ON m.id = b.media_id "
        "LEFT JOIN magazine mg ON m.id = mg.media_id "
        "LEFT JOIN dvd d ON m.id = d.media_id "
        "LEFT JOIN audiobook a ON m.id = a.media_id "
        "ORDER BY m.id;"
    );

    txn.commit();

    std::vector<std::shared_ptr<Media>> out;

    for (const auto& row : res) {
        long id = row["id"].as<long>();
        std::string title = row["title"].as<std::string>();
        bool available = row["is_available"].as<bool>();
        std::string type = row["type"].as<std::string>();

        try {
            if (type == "Book") {
                std::string author =
                    row["author"].is_null() ? "Unknown" : row["author"].as<std::string>();
                std::string publisher =
                    row["book_publisher"].is_null() ? "Unknown" : row["book_publisher"].as<std::string>();
                out.push_back(std::make_shared<Book>(id, title, author, publisher));
            }
            else if (type == "Magazine") {
                int issue = row["issue_number"].is_null() ? 0 : row["issue_number"].as<int>();
                std::string publisher =
                    row["magazine_publisher"].is_null() ? "Unknown" : row["magazine_publisher"].as<std::string>();
                out.push_back(std::make_shared<Magazine>(id, title, issue, publisher));
            }
            else if (type == "DVD") {
                std::string director =
                    row["director"].is_null() ? "Unknown" : row["director"].as<std::string>();
                out.push_back(std::make_shared<DVD>(id, title, director));
            }
            else if (type == "AudioBook") {
                std::string narrator =
                    row["narrator"].is_null() ? "Unknown" : row["narrator"].as<std::string>();
                out.push_back(std::make_shared<AudioBook>(id, title, narrator));
            }
            else {
                std::cerr << "[WARN] Unknown media type '" << type
                          << "' for media ID " << id << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Failed to build media object for id=" << id
                      << " (" << e.what() << ")" << std::endl;
        }
    }

    return out;
}


//  BORROW 
void PostgresAdapter::addActiveBorrow(int userId, long copyId) {
    pqxx::work txn(*conn_);
    txn.exec("CALL add_active_borrow($1, $2);", pqxx::params{userId, copyId});
    txn.commit();
}

void PostgresAdapter::markCopyReturned(long copyId) {
    pqxx::work txn(*conn_);
    txn.exec("CALL mark_copy_returned($1);", copyId);
    txn.commit();
}

std::optional<BorrowRecord> PostgresAdapter::findActiveBorrow(int userId, long copyId) {
    pqxx::work txn(*conn_);
    auto r = txn.exec(
        "SELECT borrow_id, user_id, copy_id, borrow_date "
        "FROM active_borrow WHERE user_id = $1 AND copy_id = $2;",
        pqxx::params{userId, copyId}
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

// USER 
int PostgresAdapter::insertUser(const std::string& name, const std::string& email,
                                const std::string& hashedPassword, const std::string& role,
                                const std::optional<std::string>& gradeLevel,
                                const std::optional<std::string>& department) {
    pqxx::work txn(*conn_);

    std::string userType = (role == "Admin") ? "ADMIN" :
                           (role == "Librarian") ? "LIBRARIAN" : "MEMBER";

    auto r = txn.exec(
        "INSERT INTO users (name, email, password_hash, user_type) "
        "VALUES ($1, $2, $3, $4) RETURNING id;",
        pqxx::params{name, email, hashedPassword, userType}
    );
    int id = r[0]["id"].as<int>();

    if (userType == "MEMBER") {
        txn.exec("INSERT INTO members (id) VALUES ($1);", pqxx::params{id});

        if (gradeLevel.has_value()) {
            txn.exec("INSERT INTO students (id, grade_level) VALUES ($1, $2);",
                            pqxx::params{id, gradeLevel.value()});
        } else if (department.has_value()) {
            txn.exec("INSERT INTO teachers (id, department) VALUES ($1, $2);",
                            pqxx::params{id, department.value()});
        }
    }

    txn.exec(
        "INSERT INTO user_roles (user_id, role_id) "
        "SELECT $1, id FROM roles WHERE name = $2;",
        pqxx::params{id, role}
    );

    txn.commit();
    return id;
}


void PostgresAdapter::assignRole(int userId, const std::string& role) {
    pqxx::work txn(*conn_);
    txn.exec(
        "INSERT INTO user_roles (user_id, role_id) "
        "SELECT $1, id FROM roles WHERE name = $2 "
        "ON CONFLICT (user_id, role_id) DO NOTHING;",
        pqxx::params{userId, role}
    );
    txn.commit();
}

std::vector<std::shared_ptr<User>> PostgresAdapter::getAllUsers() {
    pqxx::work txn(*conn_);

    auto res = txn.exec(
        "SELECT u.id AS user_id, u.name, u.email, u.user_type, "
        "       s.grade_level, t.department "
        "FROM users u "
        "LEFT JOIN members m ON u.id = m.id "
        "LEFT JOIN students s ON m.id = s.id "
        "LEFT JOIN teachers t ON m.id = t.id "
        "ORDER BY u.id;"
    );

    txn.commit();

    std::vector<std::shared_ptr<User>> users;

    for (const auto& row : res) {
        int id = row["user_id"].as<int>();
        std::string name = row["name"].as<std::string>();
        std::string email = row["email"].as<std::string>();
        std::string userType = row["user_type"].as<std::string>();

        if (userType == "ADMIN") {
            users.push_back(std::make_shared<Admin>(id, name, email));
        } 
        else if (userType == "LIBRARIAN") {
            users.push_back(std::make_shared<Librarian>(id, name, email));
        } 
        else if (userType == "MEMBER") {
            if (!row["grade_level"].is_null()) {
                std::string grade = row["grade_level"].as<std::string>();
                users.push_back(std::make_shared<Student>(id, name, email, grade));
            } 
            else if (!row["department"].is_null()) {
                std::string dept = row["department"].as<std::string>();
                users.push_back(std::make_shared<Teacher>(id, name, email, dept));
            } 
            else {
                users.push_back(std::make_shared<Member>(id, name, email));
            }
        }
    }

    return users;
}


std::optional<UserRow> PostgresAdapter::getUserByName(const std::string& username) {
    pqxx::work txn(*conn_);

    auto r = txn.exec(
        "SELECT u.id, u.name, u.email, u.password_hash, "
        "       u.user_type, "
        "       r.name AS role_name, "
        "       s.grade_level, "
        "       t.department "
        "FROM users u "
        "LEFT JOIN user_roles ur ON u.id = ur.user_id "
        "LEFT JOIN roles r ON ur.role_id = r.id "
        "LEFT JOIN members m ON u.id = m.id "
        "LEFT JOIN students s ON m.id = s.id "
        "LEFT JOIN teachers t ON m.id = t.id "
        "WHERE u.name = $1 "
        "LIMIT 1;",
        pqxx::params{username});

    if (r.empty()) {
        return std::nullopt;
    }

    const auto& row = r[0];

    int id = row["id"].as<int>();
    std::string name = row["name"].as<std::string>();
    std::string email = row["email"].as<std::string>();
    std::string passwordHash = row["password_hash"].as<std::string>();
    std::string userType = row["user_type"].as<std::string>();
    std::string roleName = row["role_name"].is_null() ? "None" : row["role_name"].as<std::string>();

    std::shared_ptr<User> user;

    // --- Determine user type based on schema ---
    if (userType == "ADMIN") {
        user = std::make_shared<Admin>(id, name, email);
    } else if (userType == "LIBRARIAN") {
        user = std::make_shared<Librarian>(id, name, email);
    } else if (userType == "MEMBER") {
        // Check subclass via member joins
        if (!row["grade_level"].is_null()) {
            std::string grade = row["grade_level"].as<std::string>();
            user = std::make_shared<Student>(id, name, email, grade);
        } else if (!row["department"].is_null()) {
            std::string dept = row["department"].as<std::string>();
            user = std::make_shared<Teacher>(id, name, email, dept);
        } else {
            user = std::make_shared<Member>(id, name, email);
        }
    } else {
        // Fallback (unexpected user_type)
        user = std::make_shared<Member>(id, name, email);
    }

    txn.commit();

    // Keep returning UserRow for auth layer
    return UserRow{id, name, email,  passwordHash, roleName};
}

