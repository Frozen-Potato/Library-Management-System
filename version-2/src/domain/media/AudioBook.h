#pragma once
#include "Media.h"

class AudioBook : public Media {
public:
    AudioBook(long id, const std::string& title, const std::string& narrator)
        : Media(id, title), narrator_(narrator) {}

    std::string getType() const override { return "AudioBook"; }

    void print(std::ostream& os) const override {
        os << "[AudioBook] " << title_ << " narrated by " << narrator_;
    }

private:
    std::string narrator_;
};
