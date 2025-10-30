#ifndef MEDIA_H
#define MEDIA_H

#include <string>
#include <iostream>

class Media {
public:
    virtual ~Media() = default;

    virtual size_t getId() const = 0;
    virtual const std::string& getName() const = 0;
    virtual void setName(std::string name) = 0;
    virtual bool getAvailability() const = 0;
    virtual void setAvailability(bool newStatus) = 0;

    virtual void print(std::ostream& out) const = 0;
};

inline std::ostream& operator<<(std::ostream& out, const Media& m) {
    m.print(out);
    return out;
}

#endif
