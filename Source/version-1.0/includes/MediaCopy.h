#ifndef MEDIA_COPY_H
#define MEDIA_COPY_H

#include <nlohmann/json.hpp>
#include <string>

enum class CopyCondition { GOOD, FAIR, DAMAGED, LOST };

inline std::string to_string(CopyCondition c) {
    switch (c) {
        case CopyCondition::GOOD: return "GOOD";
        case CopyCondition::FAIR: return "FAIR";
        case CopyCondition::DAMAGED: return "DAMAGED";
        case CopyCondition::LOST: return "LOST";
    }
    return "GOOD";
}

inline CopyCondition condition_from_string(const std::string& s) {
    if (s == "FAIR") return CopyCondition::FAIR;
    if (s == "DAMAGED") return CopyCondition::DAMAGED;
    if (s == "LOST") return CopyCondition::LOST;
    return CopyCondition::GOOD;
}

struct MediaCopy {
    int copyId{};
    int mediaId{};
    bool isAvailable{true};
    CopyCondition condition{CopyCondition::GOOD};

    nlohmann::json toJson() const {
        return {
            {"copyId", copyId},
            {"mediaId", mediaId},
            {"condition", to_string(condition)},
            {"is_available", isAvailable}
        };
    }
};

#endif