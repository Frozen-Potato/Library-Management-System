#ifndef ULT_H
#define ULT_H

#include <unordered_map>
#include "MongoLogger.h"

namespace Ultis {
    template <typename T>
    std::shared_ptr<T> findById(const std::unordered_map<int, std::shared_ptr<T>>& map, int id) {
        auto it = map.find(id);
        return it != map.end() ? it->second : nullptr;
    }

    template <typename T, typename Func>
    std::shared_ptr<T> findByName(const std::unordered_map<int, std::shared_ptr<T>>& map, std::string name, Func accessor){
        auto it = std::find_if(map.begin(), map.end(),
            [&](const auto& pair){
                return accessor(*pair.second) == name;
            }
        );
        return it != map.end() ? it->second : nullptr;
    }

    struct BorrowFunctor {
        std::shared_ptr<Member> member;

        explicit BorrowFunctor(std::shared_ptr<Member> m)
            : member(std::move(m)) {}

        void operator()(const std::shared_ptr<Media>& item) const {
            if (!member) throw GeneralFailure("No member provided");
            if (!item) throw GeneralFailure("No book provided");

            member->borrowItem(item);
        }
    };

    template <typename Container>
    void printContainer(const Container& c){
        std::for_each(c.begin(), c.end(), [](const auto& e){
            if constexpr (std::is_pointer_v<std::decay_t<decltype(e)>>){
                if (e) std::cout << *e << " ";
            } else if constexpr (requires { *e; }) {
                if (e) std::cout << *e << " ";
            } else {
                std::cout << e << " ";
            };
        });
        std::cout << "\n";
    }

    struct Result {
        bool success;
        std::string message;
        int httpCode;

        explicit operator bool() const { return success; };
    };

    template<typename Func, typename... Args>
    Result tryExecute(  MongoLogger& logger, 
                        const std::string& actionName,                     
                        const int& userId,
                        Func&& func,     
                        Args&&... args) {
        try {
            std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);

            std::string msg = actionName + " SUCCESS userId=" + std::to_string(userId);
            logger.logEvent("INFO", msg);
            return {true, msg, 200};
        }
        catch (const BookUnavailable& e) {
            std::string msg = actionName + " FAILED userId=" + std::to_string(userId);
            logger.logEvent("WARN", msg);
            return {false, e.what(), 409};
        }
        catch (const GeneralFailure& e) {
            std::string msg = actionName + " FAILED userId=" + std::to_string(userId);
            logger.logEvent("ERROR", msg);
            return {false, e.what(), 400};
        }
        catch (const std::exception& e) {
            std::string msg = actionName + " CRASHED userId=" + std::to_string(userId);
            logger.logEvent("CRITICAL", msg);
            return {false, e.what(), 500};
        }
        catch (...) {
            std::string msg = actionName + " CRASHED with unknown error userId=" + std::to_string(userId);
            logger.logEvent("CRITICAL", msg);
            return {false, "Unknown error", 500};
        };
    };
};

#endif