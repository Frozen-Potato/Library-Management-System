#include "Library.h"
#include "Book.h"
#include "Magezine.h"
#include "Ultis.h"
#include <iostream>
#include "MongoLogger.h"

static void logToMongo(const std::string& level, const std::string& message, const std::string& user = "") {
    static EnvLoader env;                     // Loaded once
    static MongoLogger logger(env.getMongoUrl());  // Created once
    try {
        logger.logEvent(level, message, user);
    } catch (const std::exception& e) {
        std::cerr << "[MongoLogger] Warning: " << e.what() << std::endl;
    }
}

Library::Library()
    : db(std::make_unique<PostgresAdapter>()) {}

Library::~Library() {}

void Library::addBook(int id, const std::string& title, const std::string& author) {
    auto book = std::make_shared<Book>(id, title, author);
    items.emplace(id, book);
    db->addBook(*book);
    
    logToMongo("INFO", "Added book: " + title);
}

void Library::addMagazine(int id, const std::string& title, int issueNumber) {
    auto mag = std::make_shared<Magazine>(id, title, issueNumber);
    items.emplace(id, mag);
    db->addMagazine(*mag);

    logToMongo("INFO", "Added magazine: " + title);
}

std::shared_ptr<Media> Library::findItemById(int id) {
    return Ultis::findById(items, id);
}

std::shared_ptr<Media> Library::findItemByName(const std::string& name) {
    return Ultis::findByName(items, name, [](const Media& m) { return m.getName(); });
}

void Library::removeItem(int id) {
    items.erase(id);
    logToMongo("WARN", "Removed item ID: " + std::to_string(id));
}

std::shared_ptr<User> Library::findUserById(int id) {
    return Ultis::findById(users, id);
}

std::shared_ptr<User> Library::findUserByName(const std::string& name) {
    return Ultis::findByName(users, name, [](const User& m){ return m.getName(); });
}

void Library::removeUser(int id) {
    users.erase(id);
    logToMongo("WARN", "Removed user ID: " + std::to_string(id));
}

std::vector<std::shared_ptr<Media>> Library::getAllMedias(){
    return db->getAllMedia();
};

void Library::borrowItem(int itemId, int userId) {
    std::scoped_lock lock(mtx);
    auto item = findItemById(itemId);
    auto user = findUserById(userId);

    if (!item || !user)
        throw GeneralFailure("Item or User not found");

    auto member = std::dynamic_pointer_cast<Member>(user);
    if (!member)
        throw GeneralFailure("This user type cannot borrow items");

    if (!item->getAvailability())
        throw BookUnavailable(item->getName() + " is already borrowed");

    member->borrowItem(item);

    BorrowRecord record(
        static_cast<int>(borrowHistory.size() + 1),
        userId,
        itemId,
        std::chrono::system_clock::now()
    );

    borrowHistory.push_back(record);
    db->addBorrowRecord(userId, itemId);

    std::string msg = member->getName() + " borrowed " + item->getName();
    std::cout << msg << std::endl;
    logToMongo("ACTION", msg, member->getName());
}

void Library::returnItem(int itemId, int userId) {
    std::scoped_lock lock(mtx);
    auto item = findItemById(itemId);
    auto user = findUserById(userId);

    if (!item || !user)
        throw GeneralFailure("Item or User not found");

    auto member = std::dynamic_pointer_cast<Member>(user);
    if (!member)
        throw GeneralFailure("This user type cannot return items");

    member->returnItem(item);

    auto it = std::find_if(borrowHistory.begin(), borrowHistory.end(),
        [&](const BorrowRecord& rec) {
            return rec.getUserId() == userId && rec.getMediaId() == itemId &&
                   !rec.getReturnDate().has_value();
        });

    if (it != borrowHistory.end()) {
        it->markReturned();
        db->markReturned(it->getBorrowId());
    }

    std::string msg = member->getName() + " returned " + item->getName();
    std::cout << msg << std::endl;

    logToMongo("ACTION", msg, member->getName());
}

void Library::removeUnavailableItems() {
    std::erase_if(items, [](const auto& pair) { return !pair.second->getAvailability(); });
}

size_t Library::countAvailableItems() {
    return std::count_if(items.begin(), items.end(), [](const auto& pair) {
        return pair.second->getAvailability();
    });
}

void Library::upperCasified() {
    std::for_each(items.begin(), items.end(), [](auto& pair) {
        std::string title = pair.second->getName();
        std::transform(title.begin(), title.end(), title.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        pair.second->setName(title);
    });
}

std::vector<std::shared_ptr<Media>> Library::getSortedByTitle() {
    std::vector<std::shared_ptr<Media>> sorted;
    sorted.reserve(items.size());

    for (const auto& [id, item] : items) {
        sorted.push_back(item);
    }

    std::sort(sorted.begin(), sorted.end(),
              [](const std::shared_ptr<Media>& a, const std::shared_ptr<Media>& b) {
                  return a->getName() < b->getName();
              });

    return sorted;
}

void Library::listItems() {
    std::for_each(items.begin(), items.end(),
                  [](const auto& pair) { std::cout << *pair.second << std::endl; });
}

void Library::listUsers() {
    std::for_each(users.begin(), users.end(),
                  [](const auto& pair) { std::cout << *pair.second << std::endl; });
}

