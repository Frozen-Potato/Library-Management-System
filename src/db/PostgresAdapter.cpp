#include "PostgresAdapter.h"

static std::chrono::system_clock::time_point parseTimestamp(const std::string& ts) {
    std::tm tm = {};
    std::istringstream ss(ts);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");  // PostgreSQL default format
    if (ss.fail()) throw std::runtime_error("Failed to parse timestamp: " + ts);
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

PostgresAdapter::PostgresAdapter()
    : conn(getConnectionFromEnv())
{
    if (!conn.is_open()) {
        throw std::runtime_error("Failed to connect to PostgreSQL database");
    }
    std::cout << "[DB] Connected to: " << conn.dbname() << std::endl;
}

PostgresAdapter::~PostgresAdapter() {
    std::cout << "[DB] Connection closed." << std::endl;
}

std::string PostgresAdapter::getConnectionFromEnv() {
    EnvLoader env;
    return env.getDatabaseUrl();
}

// --- Member operations ---
void PostgresAdapter::addMember(const Member& member) {
    pqxx::work txn(conn);

    std::string role = "MEMBER";
    if (dynamic_cast<const Student*>(&member)) {
        role = "STUDENT";
    } else if (dynamic_cast<const Teacher*>(&member)) {
        role = "TEACHER";
    }

    txn.exec(
        "INSERT INTO users (id, name, user_type) "
        "VALUES ($1, $2, 'MEMBER') "
        "ON CONFLICT (id) DO NOTHING;",
        pqxx::params(member.getId(), member.getName())
    );

    txn.exec(
        "INSERT INTO members (id, role, borrow_limit) "
        "VALUES ($1, $2, $3) "
        "ON CONFLICT (id) DO NOTHING;",
        pqxx::params(member.getId(), role, member.getBorrowLimit())
    );

    txn.commit();
}

std::vector<Member> PostgresAdapter::getAllMembers() {
    pqxx::work txn(conn);
    auto res = txn.exec(
        "SELECT u.id, u.name, m.borrow_limit "
        "FROM users u JOIN members m ON u.id = m.id;"
    );

    std::vector<Member> members;
    for (const auto& row : res) {
        members.emplace_back(
            row["id"].as<int>(),
            row["name"].as<std::string>(),
            row["borrow_limit"].as<int>()
        );
    }
    return members;
}


// --- Media operations ---
void PostgresAdapter::addBook(const Book& book) {
    pqxx::work txn(conn);
    txn.exec(
        "INSERT INTO media (id, title, author, media_type, is_available) "
        "VALUES ($1, $2, $3, 'BOOK', TRUE) "
        "ON CONFLICT (id) DO NOTHING;",
        pqxx::params(book.getId(), book.getName(), book.getAuthor())
    );
    txn.commit();
}

void PostgresAdapter::addMagazine(const Magazine& mag) {
    pqxx::work txn(conn);
    txn.exec(
        "INSERT INTO media (id, title, issue_number, media_type, is_available) "
        "VALUES ($1, $2, $3, 'MAGAZINE', TRUE) "
        "ON CONFLICT (id) DO NOTHING;",
        pqxx::params(mag.getId(), mag.getName(), mag.getIssueNumber())
    );
    txn.commit();
}

std::vector<std::shared_ptr<Media>> PostgresAdapter::getAllMedia() {
    pqxx::work txn(conn);
    auto res = txn.exec(
        "SELECT id, title, author, issue_number, is_available, media_type "
        "FROM media;"
    );

    std::vector<std::shared_ptr<Media>> items;
    for (const auto& row : res) {
        std::string type = row["media_type"].as<std::string>();
        if (type == "BOOK") {
            auto b = std::make_shared<Book>(
                row["id"].as<int>(),
                row["title"].as<std::string>(),
                row["author"].as<std::string>()
            );
            b->setAvailability(row["is_available"].as<bool>());
            items.push_back(b);
        } else {
            auto m = std::make_shared<Magazine>(
                row["id"].as<int>(),
                row["title"].as<std::string>(),
                row["issue_number"].is_null() ? 0 : row["issue_number"].as<int>()
            );
            m->setAvailability(row["is_available"].as<bool>());
            items.push_back(m);
        }
    }
    return items;
}

void PostgresAdapter::updateMediaAvailability(int mediaId, bool available) {
    pqxx::work txn(conn);
    txn.exec("UPDATE media SET is_available = $1 WHERE id = $2;",
             pqxx::params(available, mediaId));
    txn.commit();
}

// --- Borrow operations ---
void PostgresAdapter::addBorrowRecord(int userId, int mediaId) {
    pqxx::work txn(conn);
    txn.exec(
        "INSERT INTO borrow (user_id, media_id, borrow_date) "
        "VALUES ($1, $2, CURRENT_DATE);",
        pqxx::params(userId, mediaId)
    );
    txn.exec(
        "UPDATE media SET is_available = FALSE WHERE id = $1;",
        pqxx::params(mediaId)
    );
    txn.commit();
}

void PostgresAdapter::markReturned(int borrowId) {
    pqxx::work txn(conn);
    txn.exec(
        "UPDATE borrow SET return_date = CURRENT_DATE WHERE borrow_id = $1;",
        pqxx::params(borrowId)
    );
    txn.exec(
        "UPDATE media SET is_available = TRUE "
        "WHERE id = (SELECT media_id FROM borrow WHERE borrow_id = $1);",
        pqxx::params(borrowId)
    );
    txn.commit();
}

std::vector<BorrowRecord> PostgresAdapter::getAllBorrowRecords() {
    pqxx::work txn(conn);
    auto res = txn.exec(
        "SELECT borrow_id, user_id, media_id, borrow_date, return_date "
        "FROM borrow;"
    );

    std::vector<BorrowRecord> records;
    for (const auto& row : res) {
        auto borrowDate = parseTimestamp(row["borrow_date"].as<std::string>());
        std::optional<std::chrono::system_clock::time_point> returnDate = std::nullopt;
        if (!row["return_date"].is_null()) {
            returnDate = parseTimestamp(row["return_date"].as<std::string>());
        }

        records.emplace_back(
            row["borrow_id"].as<int>(),
            row["user_id"].as<int>(),
            row["media_id"].as<int>(),
            borrowDate,
            returnDate
        );
    }
    return records;
}