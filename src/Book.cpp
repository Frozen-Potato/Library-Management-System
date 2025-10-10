#include "Book.h"
#include <iostream>

using namespace std;

Book::Book() : id(0), title(""), author(""), isAvailable(true){
    ++count;
}

Book::Book(int id,const string& title,const string& author)
    : id(id), title(move(title)), author(move(author)), isAvailable(true){
    ++count;
}

Book::~Book() {
    --count;
}

Book::Book(const Book& other) 
    : id(other.id), title(other.title), author(other.author), isAvailable(other.isAvailable) {
    ++count;
}

Book::Book(Book&& other) noexcept
    : id(other.id), title(std::move(other.title)), author(std::move(other.author)), isAvailable(other.isAvailable) {
    ++count;
    std::cout << "Book moved: " << title << "\n";
    other.id = 0;
    other.isAvailable = true;
}

Book& Book::operator=(const Book& other) {
    if (this != &other) {
        id = other.id;
        title = other.title;
        author = other.author;
        isAvailable = other.isAvailable;
    }
    return *this;
}

Book& Book::operator=(Book&& other) noexcept {
    if (this != &other) {
        id = other.id;
        title = std::move(other.title);
        author = std::move(other.author);
        isAvailable = other.isAvailable;

        other.id = 0;
        other.isAvailable = true;
    }
    std::cout << "Book move-assigned: " << title << "\n";
    return *this;
}


size_t Book::getId() const {
    return static_cast<size_t>(id);
}

const std::string& Book::getName() const {
    return title;
}

void Book::setName(std::string name) {
    title = name;
}

const std::string& Book::getAuthor() const {
    return author;
}

bool Book::getAvailability() const {
    return isAvailable;
}

void Book::setAvailability(bool newStatus) {
    isAvailable = newStatus;
}

bool Book::operator==(const Book& rhs) const {
    return id == rhs.id;
}

bool Book::operator< (const Book& book) const {
    return id < book.getId();
}

void Book::print(std::ostream& out) const {
    out << "Book[ID=" << id
        << ", Title=" << title
        << ", Author=" << author
        << ", Available=" << (isAvailable ? "Yes" : "No")
        << "]";
}
