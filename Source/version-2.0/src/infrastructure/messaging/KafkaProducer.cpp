#include "KafkaProducer.h"
#include <iostream>

KafkaProducer::KafkaProducer(const std::string& brokers, const std::string& clientId) {
    std::string errstr;
    auto conf = std::unique_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
    conf->set("bootstrap.servers", brokers, errstr);
    conf->set("client.id", clientId, errstr);
    conf->set("acks", "all", errstr);
    conf->set("retries", "3", errstr);
    conf->set("linger.ms", "5", errstr);
    conf->set("compression.type", "snappy", errstr);

    producer_.reset(RdKafka::Producer::create(conf.get(), errstr));
    if (!producer_) {
        std::cerr << "[Kafka] Failed to create producer: " << errstr << std::endl;
    }
}

KafkaProducer::~KafkaProducer() {
    if (producer_) {
        producer_->flush(10000);
    }
}

bool KafkaProducer::produce(const std::string& topic,
                             const std::string& key,
                             const std::string& value) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!producer_) return false;

    RdKafka::ErrorCode err = producer_->produce(
        topic,
        RdKafka::Topic::PARTITION_UA,
        RdKafka::Producer::RK_MSG_COPY,
        const_cast<char*>(value.c_str()), value.size(),
        key.c_str(), key.size(),
        0, nullptr);

    if (err != RdKafka::ERR_NO_ERROR) {
        std::cerr << "[Kafka] Produce failed: " << RdKafka::err2str(err) << std::endl;
        return false;
    }

    producer_->poll(0);
    return true;
}

bool KafkaProducer::produceJson(const std::string& topic,
                                 const std::string& key,
                                 const nlohmann::json& value) {
    return produce(topic, key, value.dump());
}

void KafkaProducer::flush(int timeoutMs) {
    if (producer_) {
        producer_->flush(timeoutMs);
    }
}
