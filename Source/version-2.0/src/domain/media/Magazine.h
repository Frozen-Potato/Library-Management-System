#pragma once
#include "Media.h"

class Magazine : public Media {
public:
    Magazine(long id, const std::string& title, int issueNumber, const std::string& publisher)
        : Media(id, title), issueNumber_(issueNumber), publisher_(publisher) {}

    int getIssueNumber() const { return issueNumber_; }
    const std::string& getPublisher() const { return publisher_; }

    std::string getType() const override { return "Magazine"; }

    void print(std::ostream& os) const override {
        os << "[Magazine] " << title_ << " (Issue " << issueNumber_ << ") - " << publisher_;
    }

private:
    int issueNumber_;
    std::string publisher_;
};
