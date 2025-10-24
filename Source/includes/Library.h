#ifndef LIBRARY_H
#define LIBRARY_H

#include "User.h"
#include "Media.h"
#include "Exceptions.h"
#include "Member.h"
#include "BorrowRecord.h"
#include "PostgresAdapter.h"

#include <algorithm>
#include <unordered_map>
#include <memory>
#include <vector>
#include <mutex>

class Library {
private:
    std::unordered_map<int, std::shared_ptr<Media>> items;
    std::unordered_map<int, std::shared_ptr<User>> users;
    std::vector<BorrowRecord> borrowHistory;
    mutable std::mutex mtx;

    std::unique_ptr<PostgresAdapter> db; 

public:
    Library();
    ~Library();

    void addBook(int id, const std::string& title, const std::string& author);
    void addMagazine(int id, const std::string& title, int issueNumber);

    std::shared_ptr<Media> findItemById(int id);
    std::shared_ptr<Media> findItemByName(const std::string& name);
    void removeItem(int id);

    template<typename T, typename... Args>
    void addUser(int id, Args&&... args) {
        auto user = std::make_shared<T>(std::forward<Args>(args)...);
        users.emplace(id, user);

        if constexpr (std::is_base_of<Member, T>::value) {
            db->addMember(*user);  // insert into DB
        }
    }

    std::shared_ptr<User> findUserById(int id);
    std::shared_ptr<User> findUserByName(const std::string& name);
    void removeUser(int id);

    std::vector<std::shared_ptr<Media>> getAllMedias();

    void borrowItem(int itemId, int userId);
    void returnItem(int itemId, int userId);
    void removeUnavailableItems();
    void upperCasified();

    size_t countAvailableItems();

    std::vector<std::shared_ptr<Media>> getSortedByTitle();

    void listItems();
    void listUsers();
};

#endif
