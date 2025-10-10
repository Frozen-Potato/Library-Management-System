#ifndef POSTGRES_ADAPTER_H
#define POSTGRES_ADAPTER_H

#include <pqxx/pqxx>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>

#include "Member.h"
#include "Media.h"
#include "Book.h"
#include "Magezine.h"
#include "BorrowRecord.h"
#include "EnvLoader.h"
#include "Student.h"
#include "Teacher.h"

class PostgresAdapter {
private:
    pqxx::connection conn;
    std::string getConnectionFromEnv();

public:
    PostgresAdapter();
    ~PostgresAdapter();

    // --- Member operations ---
    void addMember(const Member& member);
    std::vector<Member> getAllMembers();

    // --- Media operations ---
    void addBook(const Book& book);
    void addMagazine(const Magazine& mag);
    std::vector<std::shared_ptr<Media>> getAllMedia();
    void updateMediaAvailability(int mediaId, bool available);

    // --- Borrow operations ---
    void addBorrowRecord(int userId, int mediaId);
    void markReturned(int borrowId);
    std::vector<BorrowRecord> getAllBorrowRecords();
};

#endif
