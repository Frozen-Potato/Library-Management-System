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
};

void PostgresAdapter::attachBook(long mediaId, const std::string& author, const std::string& isbn) {
    pqxx::work txn(*conn_);
    txn.exec(
        "INSERT INTO book (media_id, author, isbn) VALUES ($1, $2, $3) "
        "ON CONFLICT (media_id) DO NOTHING;",
        pqxx::params{mediaId, author, isbn}
    );
    txn.commit();
};

void PostgresAdapter::attachMagazine(long mediaId, int issueNumber, const std::string& publisher) {
    pqxx::work txn(*conn_);
    txn.exec(
        "INSERT INTO magazine (media_id, issue_number, publisher) VALUES ($1, $2, $3) "
        "ON CONFLICT (media_id) DO NOTHING;",
        pqxx::params{mediaId, issueNumber, publisher}
    );
    txn.commit();
};

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
};

MediaCopy PostgresAdapter::getCopy(long copyId) {
    pqxx::work txn(*conn_);
    auto r = txn.exec(
        "SELECT copy_id, media_id, condition, is_available "
        "FROM media_copy WHERE copy_id = $1;",
        pqxx::params{copyId}
    );
    txn.commit();
    if (r.empty()) throw std::runtime_error("Copy not found.");

    MediaCopy c;
    c.copyId = r[0]["copy_id"].as<long>();
    c.mediaId = r[0]["media_id"].as<long>();
    c.condition = condition_from_string(r[0]["condition"].as<std::string>());
    c.isAvailable = r[0]["is_available"].as<bool>();
    return c;
};

std::vector<MediaCopy> PostgresAdapter::listCopiesByMedia(long mediaId) {
    pqxx::work txn(*conn_);
    auto res = txn.exec(
        "SELECT copy_id, media_id, condition, is_available "
        "FROM media_copy WHERE media_id = $1 ORDER BY copy_id;",
        pqxx::params{mediaId}
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
};

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
                std::string isbn =
                    row["isbn"].is_null() ? "Unknown" : row["isbn"].as<std::string>();
                out.push_back(std::make_shared<Book>(id, title, author, isbn));
            }
            else if (type == "Magazine") {
                int issue = row["issue_number"].is_null() ? 0 : row["issue_number"].as<int>();
                std::string publisher =
                    row["publisher"].is_null() ? "Unknown" : row["publisher"].as<std::string>();
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
};

//  BORROW 
void PostgresAdapter::addActiveBorrow(int userId, long copyId) {
    pqxx::work txn(*conn_);
    txn.exec("CALL add_active_borrow($1, $2);", pqxx::params{userId, copyId});
    txn.commit();
}

void PostgresAdapter::markCopyReturned(long copyId) {
    pqxx::work txn(*conn_);
    txn.exec("CALL mark_copy_returned($1);", pqxx::params{copyId});
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
};

// USER 
int PostgresAdapter::insertUser(const std::string& name, const std::string& email,
                                const std::string& hashedPassword, const std::string& role,
                                const std::optional<std::string>& gradeLevel,
                                const std::optional<std::string>& department) {
    pqxx::work txn(*conn_);

    // Normalize role to uppercase for comparison
    std::string normalizedRole = role;
    std::transform(normalizedRole.begin(), normalizedRole.end(), normalizedRole.begin(), ::toupper);

    // Insert into users table (no user_type column)
    auto r = txn.exec(
        "INSERT INTO users (name, email, password_hash) "
        "VALUES ($1, $2, $3) RETURNING id;",
        pqxx::params{name, email, hashedPassword}
    );
    int id = r[0]["id"].as<int>();

    // Insert into appropriate subtype table based on role
    if (normalizedRole == "ADMIN") {
        txn.exec("INSERT INTO admins (id) VALUES ($1);", pqxx::params{id});
    } 
    else if (normalizedRole == "LIBRARIAN") {
        txn.exec("INSERT INTO librarians (id) VALUES ($1);", pqxx::params{id});
    } 
    else if (normalizedRole == "STUDENT" || normalizedRole == "TEACHER" || normalizedRole == "MEMBER") {
        // All students, teachers, and members go into members table first
        txn.exec("INSERT INTO members (id) VALUES ($1);", pqxx::params{id});

        // Then insert into appropriate subtype
        if (normalizedRole == "STUDENT") {
            std::string grade = gradeLevel.value_or("Unknown");
            txn.exec("INSERT INTO students (id, grade_level) VALUES ($1, $2);",
                     pqxx::params{id, grade});
        } 
        else if (normalizedRole == "TEACHER") {
            std::string dept = department.value_or("Unknown");
            txn.exec("INSERT INTO teachers (id, department) VALUES ($1, $2);",
                     pqxx::params{id, dept});
        }
    }

    // Assign role in user_roles table
    txn.exec(
        "INSERT INTO user_roles (user_id, role_id) "
        "SELECT $1, id FROM roles WHERE name = $2;",
        pqxx::params{id, normalizedRole}
    );

    txn.commit();
    return id;
};


void PostgresAdapter::assignRole(int userId, const std::string& role) {
    pqxx::work txn(*conn_);
    txn.exec(
        "INSERT INTO user_roles (user_id, role_id) "
        "SELECT $1, id FROM roles WHERE name = $2 "
        "ON CONFLICT (user_id, role_id) DO NOTHING;",
        pqxx::params{userId, role}
    );
    txn.commit();
};

std::vector<std::shared_ptr<User>> PostgresAdapter::getAllUsers() {
    pqxx::work txn(*conn_);

    auto res = txn.exec(
        "SELECT u.id AS user_id, u.name, u.email, "
        "       CASE "
        "         WHEN a.id IS NOT NULL THEN 'ADMIN' "
        "         WHEN l.id IS NOT NULL THEN 'LIBRARIAN' "
        "         WHEN t.id IS NOT NULL THEN 'TEACHER' "
        "         WHEN s.id IS NOT NULL THEN 'STUDENT' "
        "         ELSE 'MEMBER' "
        "       END AS user_type, "
        "       s.grade_level, t.department "
        "FROM users u "
        "LEFT JOIN admins a ON u.id = a.id "
        "LEFT JOIN librarians l ON u.id = l.id "
        "LEFT JOIN members m ON u.id = m.id "
        "LEFT JOIN students s ON m.id = s.id "
        "LEFT JOIN teachers t ON m.id = t.id "
        "ORDER BY u.id;"
    );
    txn.commit();

    auto col_user_id    = res.column_number("user_id");
    auto col_name       = res.column_number("name");
    auto col_email      = res.column_number("email");
    auto col_user_type  = res.column_number("user_type");
    auto col_grade      = res.column_number("grade_level");
    auto col_department = res.column_number("department");   

    std::vector<std::shared_ptr<User>> users;
    users.reserve(res.size());

    for (const auto& row : res) {
        int id = row[col_user_id].as<int>();
        std::string name = row[col_name].as<std::string>();
        std::string email = row[col_email].as<std::string>();
        std::string userType = row[col_user_type].as<std::string>();

        if (userType == "ADMIN") {
            users.push_back(std::make_shared<Admin>(id, name, email));
        }
        else if (userType == "LIBRARIAN") {
            users.push_back(std::make_shared<Librarian>(id, name, email));
        }
        else if (userType == "TEACHER") {
            std::string dept = row[col_department].is_null() ? "" : row[col_department].as<std::string>();
            users.push_back(std::make_shared<Teacher>(id, name, email, dept));
        }
        else if (userType == "STUDENT") {
            std::string grade = row[col_grade].is_null() ? "" : row[col_grade].as<std::string>();
            users.push_back(std::make_shared<Student>(id, name, email, grade));
        }
        else {
            users.push_back(std::make_shared<Member>(id, name, email));
        }
    }
    return users;
};



std::optional<UserRow> PostgresAdapter::getUserByName(const std::string& username) {
    pqxx::work txn(*conn_);

    auto r = txn.exec(
        "SELECT u.id, u.name, u.email, u.password_hash, "
        "       CASE "
        "         WHEN a.id IS NOT NULL THEN 'ADMIN' "
        "         WHEN l.id IS NOT NULL THEN 'LIBRARIAN' "
        "         WHEN t.id IS NOT NULL THEN 'TEACHER' "
        "         WHEN s.id IS NOT NULL THEN 'STUDENT' "
        "         ELSE 'MEMBER' "
        "       END AS role "
        "FROM users u "
        "LEFT JOIN admins a ON u.id = a.id "
        "LEFT JOIN librarians l ON u.id = l.id "
        "LEFT JOIN members m ON u.id = m.id "
        "LEFT JOIN teachers t ON m.id = t.id "
        "LEFT JOIN students s ON m.id = s.id "
        "WHERE u.name = $1 "
        "LIMIT 1;",
        pqxx::params{username});

    txn.commit();

    if (r.empty()) {
        return std::nullopt;
    }

    const auto& row = r[0];

    return UserRow{
        row["id"].as<int>(),
        row["name"].as<std::string>(),
        row["email"].as<std::string>(),
        row["password_hash"].as<std::string>(),
        row["role"].as<std::string>()
    };
};

std::optional<UserRow> PostgresAdapter::getUserByEmail(const std::string& email) {
    pqxx::work txn(*conn_);
    auto res = txn.exec(
        "SELECT u.id, u.name, u.email, u.password_hash, "
        "       CASE "
        "         WHEN a.id IS NOT NULL THEN 'ADMIN' "
        "         WHEN l.id IS NOT NULL THEN 'LIBRARIAN' "
        "         WHEN t.id IS NOT NULL THEN 'TEACHER' "
        "         WHEN s.id IS NOT NULL THEN 'STUDENT' "
        "         ELSE 'MEMBER' "
        "       END AS role "
        "FROM users u "
        "LEFT JOIN admins a ON u.id = a.id "
        "LEFT JOIN librarians l ON u.id = l.id "
        "LEFT JOIN members m ON u.id = m.id "
        "LEFT JOIN teachers t ON m.id = t.id "
        "LEFT JOIN students s ON m.id = s.id "
        "WHERE u.email = $1;",
        pqxx::params{email});

    txn.commit();

    if (res.empty()) {
        return std::nullopt;
    }

    const auto& row = res[0];

    return UserRow{
        row["id"].as<int>(),
        row["name"].as<std::string>(),
        row["email"].as<std::string>(),
        row["password_hash"].as<std::string>(),
        row["role"].as<std::string>()
    };
};

std::optional<UserRow> PostgresAdapter::getUserById(const int userId) {
    pqxx::work txn(*conn_);
    auto res = txn.exec(
        "SELECT u.id, u.name, u.email, u.password_hash, "
        "       CASE "
        "         WHEN a.id IS NOT NULL THEN 'ADMIN' "
        "         WHEN l.id IS NOT NULL THEN 'LIBRARIAN' "
        "         WHEN t.id IS NOT NULL THEN 'TEACHER' "
        "         WHEN s.id IS NOT NULL THEN 'STUDENT' "
        "         ELSE 'MEMBER' "
        "       END AS role "
        "FROM users u "
        "LEFT JOIN admins a ON u.id = a.id "
        "LEFT JOIN librarians l ON u.id = l.id "
        "LEFT JOIN members m ON u.id = m.id "
        "LEFT JOIN teachers t ON m.id = t.id "
        "LEFT JOIN students s ON m.id = s.id "
        "WHERE u.id = $1;",
        pqxx::params{userId});

    txn.commit();

    if (res.empty()) {
        return std::nullopt;
    }

    const auto& row = res[0];

    return UserRow{
        row["id"].as<int>(),
        row["name"].as<std::string>(),
        row["email"].as<std::string>(),
        row["password_hash"].as<std::string>(),
        row["role"].as<std::string>()
    };
};

std::vector<std::tuple<std::string, std::string, std::string>> PostgresAdapter::getAllRolePermissions() {

    pqxx::work txn(*conn_);
    auto res = txn.exec(
        R"(
            SELECT r.name AS role, p.table_name, p.action
            FROM role_permissions rp
            JOIN roles r ON rp.role_id = r.id
            JOIN permissions p ON rp.permission_id = p.id;
        )"
    );
    txn.commit();

    std::vector<std::tuple<std::string, std::string, std::string>> perms;
    perms.reserve(res.size());
    for (const auto& row : res) {
        perms.emplace_back(row["role"].as<std::string>(),
                           row["table_name"].as<std::string>(),
                           row["action"].as<std::string>());
    }
    return perms;
}


