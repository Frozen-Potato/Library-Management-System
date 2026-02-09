#pragma once
#include <string>
#include <mutex>
#include <functional>
#include <nlohmann/json.hpp>
#include <librdkafka/rdkafkacpp.h>

class KafkaProducer {
public:
    KafkaProducer(const std::string& brokers, const std::string& clientId = "library-producer");
    ~KafkaProducer();

    bool produce(const std::string& topic,
                 const std::string& key,
                 const std::string& value);
    bool produceJson(const std::string& topic,
                     const std::string& key,
                     const nlohmann::json& value);
    void flush(int timeoutMs = 5000);

private:
    std::unique_ptr<RdKafka::Producer> producer_;
    std::mutex mtx_;
};
