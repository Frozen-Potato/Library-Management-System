#include "Magezine.h"
#include <iostream>
#include <utility>

using namespace std;

Magazine::Magazine() : id(0), title(""), issueNumber(0), available(true) {
    ++count;
}

Magazine::Magazine(int id, const string& title, int issueNumber)
    : id(id), title(move(title)), issueNumber(issueNumber), available(true) {
    ++count;
}

Magazine::~Magazine() {
    --count;
}

Magazine::Magazine(const Magazine& other)
    : id(other.id), title(other.title), issueNumber(other.issueNumber), available(other.available) {
    ++count;
}

Magazine::Magazine(Magazine&& other) noexcept
    : id(other.id), title(std::move(other.title)), issueNumber(other.issueNumber), available(other.available) {
    ++count;
    cout << "Magazine moved: " << title << "\n";
    other.id = 0;
    other.issueNumber = 0;
    other.available = true;
}

Magazine& Magazine::operator=(const Magazine& other) {
    if (this != &other) {
        id = other.id;
        title = other.title;
        issueNumber = other.issueNumber;
        available = other.available;
    }
    return *this;
}

Magazine& Magazine::operator=(Magazine&& other) noexcept {
    if (this != &other) {
        id = other.id;
        title = std::move(other.title);
        issueNumber = other.issueNumber;
        available = other.available;

        other.id = 0;
        other.issueNumber = 0;
        other.available = true;
    }
    cout << "Magazine move-assigned: " << title << "\n";
    return *this;
}

// Media interface implementations
size_t Magazine::getId() const {
    return static_cast<size_t>(id);
}

const string& Magazine::getName() const {
    return title;
}

void Magazine::setName(string name) {
    title = std::move(name);
}

bool Magazine::getAvailability() const {
    return available;
}

void Magazine::setAvailability(bool newStatus) {
    available = newStatus;
}

int Magazine::getIssueNumber() const {
    return issueNumber;
}

// Comparison
bool Magazine::operator==(const Magazine& rhs) const {
    return id == rhs.id;
}

bool Magazine::operator<(const Magazine& rhs) const {
    return id < rhs.id;
}

// Printing
void Magazine::print(ostream& out) const {
    out << "Magazine[ID=" << id
        << ", Title=" << title
        << ", Issue=" << issueNumber
        << ", Available=" << (available ? "Yes" : "No")
        << "]";
}
