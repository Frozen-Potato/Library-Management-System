#pragma once
#include <chrono>
#include <optional>

class BorrowRecord {
public:
    BorrowRecord(long id, int userId, long copyId,
                 std::chrono::system_clock::time_point borrowDate,
                 std::optional<std::chrono::system_clock::time_point> returnDate = std::nullopt)
        : borrowId_(id), userId_(userId), copyId_(copyId),
          borrowDate_(borrowDate), returnDate_(returnDate) {}

    long getBorrowId() const { return borrowId_; }
    int getUserId() const { return userId_; }
    long getCopyId() const { return copyId_; }
    auto getBorrowDate() const { return borrowDate_; }
    auto getReturnDate() const { return returnDate_; }

    void markReturned() {
        returnDate_ = std::chrono::system_clock::now();
    }

private:
    long borrowId_;
    int userId_;
    long copyId_;
    std::chrono::system_clock::time_point borrowDate_;
    std::optional<std::chrono::system_clock::time_point> returnDate_;
};
