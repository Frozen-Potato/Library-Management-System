#ifndef ULT_H
#define ULT_H

#include <unordered_map>

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
}

#endif

