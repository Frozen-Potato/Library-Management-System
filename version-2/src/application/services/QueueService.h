#pragma once
#include <nlohmann/json.hpp>
#include <string>

class QueueService {
public:
    virtual ~QueueService() = default;
    virtual void enqueue(const std::string& taskType,
                         const nlohmann::json& payload) = 0;
};
