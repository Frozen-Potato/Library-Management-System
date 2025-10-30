#pragma once
#include <string>
#include <memory>
#include <iostream>

class Media {
public:
    Media(long id, std::string title, bool available = true)
        : id_(id), title_(std::move(title)), isAvailable_(available) {}
    virtual ~Media() = default;

    long getId() const { return id_; }
    const std::string& getTitle() const { return title_; }
    bool isAvailable() const { return isAvailable_; }
    void setAvailable(bool available) { isAvailable_ = available; }
    void setTitle(const std::string& t) { title_ = t; }

    virtual std::string getType() const = 0;
    virtual void print(std::ostream& os) const = 0;

protected:
    long id_;
    std::string title_;
    bool isAvailable_;
};