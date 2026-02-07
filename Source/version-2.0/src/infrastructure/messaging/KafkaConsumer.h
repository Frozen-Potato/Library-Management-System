#pragma once
#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <thread>
#include <nlohmann/json.hpp>
#include <librdkafka/rdkafkacpp.h>

struct KafkaMessage {
    std::string topic;
    std::string key;
    std::string value;
    int64_t offset;
    int32_t partition;
};

class KafkaConsumer {
public:
    using MessageHandler = std::function<void(const KafkaMessage&)>;

    KafkaConsumer(const std::string& brokers,
                  const std::string& groupId,
                  const std::vector<std::string>& topics);
    ~KafkaConsumer();

    void start(MessageHandler handler);
    void stop();

private:
    std::unique_ptr<RdKafka::KafkaConsumer> consumer_;
    std::atomic<bool> running_;
    std::thread thread_;
};
