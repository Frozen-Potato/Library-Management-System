#ifndef MAGAZINE_H
#define MAGAZINE_H

#include "Media.h"
#include <string>

class Magazine : public Media {
private:
    int id;
    std::string title;
    int issueNumber;
    bool available;

public:
    inline static size_t count = 0;

    Magazine();
    Magazine(int id, const std::string& title, int issueNumber);

    ~Magazine();
    Magazine(const Magazine& other);
    Magazine(Magazine&& other) noexcept;
    Magazine& operator=(const Magazine& other);
    Magazine& operator=(Magazine&& other) noexcept;

    size_t getId() const override;
    const std::string& getName() const override;
    void setName(std::string name) override;
    bool getAvailability() const override;
    void setAvailability(bool newStatus) override;

    int getIssueNumber() const;

    bool operator==(const Magazine& rhs) const;
    bool operator<(const Magazine& rhs) const;

    void print(std::ostream& out) const override;
};

#endif
