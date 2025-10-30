#include "Member.h"
#include "Exceptions.h"
#include <iostream>

Member::Member(int id, std::string name, int limit)
        : User(id, std::move(name)), borrowLimit(limit) {}

size_t Member::getId() const {
    return static_cast<size_t>(id);
}

const std::string& Member::getName() const {
    return name;
}

void Member::setName(std::string newName) {
    name = std::move(newName);
}

int Member::getBorrowLimit() const {
    return 3;
}

void Member::borrowItem(const std::shared_ptr<Media>& item) {
    if (!item->getAvailability()) {
        throw BookUnavailable("Unavailable: " + item->getName());
    }

    if (borrowedItems.size() >= static_cast<size_t>(getBorrowLimit())) {
        throw BorrowLimitExceeded(name + " cannot borrow more items");
    }

    item->setAvailability(false);
    borrowedItems.push_back(item->getId());
    std::cout << name << " borrowed " << *item << std::endl;
}

void Member::returnItem(std::shared_ptr<Media>& item) {
    auto it = std::find(borrowedItems.begin(), borrowedItems.end(), item->getId());
    if (it != borrowedItems.end()) {
        borrowedItems.erase(it);
        item->setAvailability(true);
        std::cout << name << " returned " << *item << std::endl;
    } else {
        throw GeneralFailure("Item not borrowed: " + item->getName());
    }
}

bool Member::allBorrowedUnavailable(const std::unordered_map<int, std::shared_ptr<Media>>& libItems) const {
    return std::all_of(borrowedItems.begin(), borrowedItems.end(), [&](int id) {
        auto it = libItems.find(id);
        return it != libItems.end() && !it->second->getAvailability();
    });
}

bool Member::hasBorrowed(int itemId) const {
    return std::any_of(borrowedItems.begin(), borrowedItems.end(),
                       [itemId](int id) { return id == itemId; });
}

bool Member::operator==(const Member& person) const {
    return name == person.getName();
}

void Member::print(std::ostream& out) const {
    out << "Member[ID=" << id << ", Name=" << name << "]";
}
