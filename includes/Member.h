#ifndef MEMBER_H
#define MEMBER_H

#include "Media.h"
#include "User.h"
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <ostream>
#include <chrono>

class Member : public User {
private:
    std::vector<int> borrowedItems;
    int borrowLimit;

public:
    Member(int id, std::string name, int limit);
    ~Member() override = default;

    int getBorrowLimit() const;

    void borrowItem(const std::shared_ptr<Media>& item);
    void returnItem(std::shared_ptr<Media>& item);

    bool hasBorrowed(int itemId) const;
    bool allBorrowedUnavailable(const std::unordered_map<int, std::shared_ptr<Media>>& libItems) const;

    size_t getId() const override;
    const std::string& getName() const override;
    void setName(std::string name) override;

    void print(std::ostream& out) const override;

    bool operator==(const Member& person) const;
};

#endif
