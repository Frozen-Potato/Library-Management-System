#pragma once
#include "Media.h"

class Book : public Media {
public:
    Book(long id, const std::string& title, const std::string& author, const std::string& isbn)
        : Media(id, title), author_(author), isbn_(isbn) {}

    std::string getAuthor() const { return author_; }
    std::string getIsbn() const { return isbn_; }

    std::string getType() const override { return "Book"; }

    void print(std::ostream& os) const override {
        os << "[Book] " << title_ << " by " << author_ << " (ISBN: " << isbn_ << ")";
    }

private:
    std::string author_;
    std::string isbn_;
};