#pragma once
#include "Media.h"

class DVD : public Media {
public:
    DVD(long id, const std::string& title, const std::string& director)
        : Media(id, title), director_(director) {}

    std::string getType() const override { return "DVD"; }

    void print(std::ostream& os) const override {
        os << "[DVD] " << title_ << " directed by " << director_;
    }

private:
    std::string director_;
};
