#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <tuple>
#include <memory>
#include "src/data/PostgresAdapter.h"

class PermissionService {
public:
    explicit PermissionService(std::shared_ptr<PostgresAdapter> db)
        : db_(std::move(db)) {
        loadCache();
    }

    bool hasPermission(const std::string& role,
                       const std::string& table,
                       const std::string& action) const {
        auto key = makeKey(role, table, action);
        return cache_.find(key) != cache_.end();
    }

    void reload() { loadCache(); }

private:
    std::shared_ptr<PostgresAdapter> db_;
    std::unordered_set<std::string> cache_;  

    void loadCache() {
        cache_.clear();
        for (auto& [role, table, action] : db_->getAllRolePermissions()) {
            cache_.insert(makeKey(role, table, action));
        }
        std::cout << "[PermissionService] Cached " << cache_.size() << " permissions.\n";
    }

    static std::string makeKey(const std::string& role,
                               const std::string& table,
                               const std::string& action) {
        return role + ":" + table + ":" + action;
    }
};
