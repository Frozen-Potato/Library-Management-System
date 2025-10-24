#ifndef USER_H
#define USER_H

#include <string>
#include <chrono>

class User {
    protected:
        int id{};
        std::string name;
        std::chrono::system_clock::time_point createdAt;
    public:
        User(int id, std::string name) : id(id), name(std::move(name)), createdAt(std::chrono::system_clock::now()) {}
        virtual ~User() = default;
        virtual size_t getId() const = 0;
        virtual const std::string& getName() const = 0;
        virtual void setName(std::string name) = 0;

        virtual void print(std::ostream& out) const = 0;

        std::string getCreationTime() const {
            std::time_t t = std::chrono::system_clock::to_time_t(createdAt);
            return std::ctime(&t);
        }
};

inline std::ostream& operator<<(std::ostream& out, const User& u){
    u.print(out);
    return out;
}

#endif