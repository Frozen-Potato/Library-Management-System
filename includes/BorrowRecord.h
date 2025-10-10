#ifndef BORROW_RECORD_H
#define BORROW_RECORD_H

#include <string>
#include <optional>
#include <chrono>
#include <iostream>
#include <cstring>

class BorrowRecord {
private:
    int borrowId;
    int userId;
    int mediaId;
    std::chrono::system_clock::time_point borrowDate;
    std::optional<std::chrono::system_clock::time_point> returnDate;

public:
    BorrowRecord(
        int borrowId,
        int userId,
        int mediaId,
        const std::chrono::system_clock::time_point& borrowDate,
        const std::optional<std::chrono::system_clock::time_point>& returnDate = std::nullopt)
        : borrowId(borrowId),
          userId(userId),
          mediaId(mediaId),
          borrowDate(borrowDate),
          returnDate(returnDate) {}

    int getBorrowId() const { return borrowId; }
    int getUserId() const { return userId; }
    int getMediaId() const { return mediaId; }

    std::chrono::system_clock::time_point getBorrowDate() const { return borrowDate; }
    std::optional<std::chrono::system_clock::time_point> getReturnDate() const { return returnDate; }

    friend std::ostream& operator<<(std::ostream& out, const BorrowRecord& record);

    void markReturned() {
        returnDate = std::chrono::system_clock::now();
    }
};

inline std::ostream& operator<<(std::ostream& out, const BorrowRecord& record) {
    auto toTimeString = [](const auto& tp) {
        std::time_t t = std::chrono::system_clock::to_time_t(tp);
        char buf[26];
        #ifdef _WIN32
            ctime_s(buf, sizeof(buf), &t);  // Windows
        #else
            ctime_r(&t, buf);               // POSIX 
        #endif
        buf[strcspn(buf, "\n")] = '\0';
        return std::string(buf);
    };

    out << "BorrowRecord[ID=" << record.borrowId
        << ", UserID=" << record.userId
        << ", MediaID=" << record.mediaId
        << ", BorrowDate=" << toTimeString(record.borrowDate);

    if (record.returnDate)
        out << ", ReturnDate=" << toTimeString(*record.returnDate);
    else
        out << ", ReturnDate=None";

    out << "]";
    return out;
}

#endif
