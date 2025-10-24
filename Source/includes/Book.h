#ifndef BOOK_H
#define BOOK_H

#include <string>
#include "Media.h"

class Book: public Media {
    private:
        int id;
        std::string title;
        std::string author;
        bool isAvailable;
    public:
        inline static size_t count = 0;

        Book();
        explicit Book(int id,const std::string& title,const std::string& author);
        Book(const Book& other);
        Book(Book&& other) noexcept;
        Book& operator=(const Book& other);                                                                                                                                                                                                                                     
        Book& operator=(Book&& other) noexcept;
        ~Book();

        size_t getId() const override;
        const std::string& getName() const override;
        void setName(std::string name) override;
        const std::string& getAuthor() const;
        bool getAvailability() const override;
        void setAvailability(bool newStatus) override;
        bool operator== (const Book& rhs) const;
        bool operator< (const Book& book) const;
        void print(std::ostream& out) const override;
        friend std::ostream& operator<<(std::ostream& out, const Book& book);

};

#endif