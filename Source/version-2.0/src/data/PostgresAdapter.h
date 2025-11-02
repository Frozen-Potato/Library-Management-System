#pragma once
#include <memory>
#include <optional>
#include <vector>
#include <string>
#include <unordered_set>
#include "pqxx/pqxx"

#include "src/domain/media/MediaCopy.h"
#include "src/domain/borrow/BorrowRecord.h"
#include "src/domain/user/User.h"
#include "src/domain/media/MediaCopy.h"
#include "src/utils/DateTimeUtils.h"
#include "src/domain/user/User.h"
#include "src/domain/user/Admin.h"
#include "src/domain/user/Member.h"
#include "src/domain/user/Student.h"
#include "src/domain/user/Teacher.h"
#include "src/domain/user/Librarian.h"
#include "src/domain/media/Media.h"
#include "src/domain/media/Book.h"
#include "src/domain/media/Magazine.h"
#include "src/domain/media/DVD.h"
#include "src/domain/media/AudioBook.h"


// Simple DTO for Auth queries
struct UserRow {
    int id;
    std::string username;
    std::string email;
    std::string hashedPassword;
    std::string role;
};

class PostgresAdapter {
public:
    explicit PostgresAdapter(std::shared_ptr<pqxx::connection> conn);

    // Media & Copies
    long createMedia(int mediaTypeId, const std::string& title);
    void attachBook(long mediaId, const std::string& author, const std::string& isbn);
    void attachMagazine(long mediaId, int issueNumber, const std::string& publisher);
    MediaCopy createMediaCopy(long mediaId, const std::string& condition);
    MediaCopy getCopy(long copyId);
    std::vector<MediaCopy> listCopiesByMedia(long mediaId);
    std::vector<std::shared_ptr<class Media>> getAllMedia();

    // Borrowing / Returning
    void addActiveBorrow(int userId, long copyId);
    void markCopyReturned(long copyId);
    std::optional<BorrowRecord> findActiveBorrow(int userId, long copyId);

    // User Management
    int insertUser( const std::string& name, const std::string& email,
                    const std::string& hashedPassword, const std::string& role,
                    const std::optional<std::string>& gradeLevel,
                    const std::optional<std::string>& department);
    void assignRole(int userId, const std::string& role);
    std::vector<std::shared_ptr<User>> getAllUsers();
    std::optional<UserRow> getUserByName(const std::string& username);

    // Permissions
    std::vector<std::tuple<std::string, std::string, std::string>> getAllRolePermissions();
    
private:
    std::shared_ptr<pqxx::connection> conn_;
};
