#pragma once
#include <string>
#include <iostream>

enum class CopyCondition {
    GOOD,
    FAIR,
    DAMAGED,
    LOST
};

inline std::string to_string(CopyCondition c) {
    switch (c) {
        case CopyCondition::GOOD: return "GOOD";
        case CopyCondition::FAIR: return "FAIR";
        case CopyCondition::DAMAGED: return "DAMAGED";
        case CopyCondition::LOST: return "LOST";
    }
    return "UNKNOWN";
}

inline CopyCondition condition_from_string(const std::string& s) {
    if (s == "GOOD") return CopyCondition::GOOD;
    if (s == "FAIR") return CopyCondition::FAIR;
    if (s == "DAMAGED") return CopyCondition::DAMAGED;
    if (s == "LOST") return CopyCondition::LOST;
    return CopyCondition::GOOD;
}

struct MediaCopy {
    long copyId{};
    long mediaId{};
    CopyCondition condition{CopyCondition::GOOD};
    bool isAvailable{true};

    void print(std::ostream& os) const {
        os << "Copy #" << copyId << " of media " << mediaId
           << " [" << to_string(condition) << "] "
           << (isAvailable ? "(Available)" : "(Borrowed)");
    }
};
