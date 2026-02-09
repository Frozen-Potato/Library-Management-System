#include "KafkaConsumer.h"
#include <iostream>

KafkaConsumer::KafkaConsumer(const std::string& brokers,
                             const std::string& groupId,
                             const std::vector<std::string>& topics)
    : running_(false) {
    std::string errstr;
    auto conf = std::unique_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
    conf->set("bootstrap.servers", brokers, errstr);
    conf->set("group.id", groupId, errstr);
    conf->set("auto.offset.reset", "earliest", errstr);
    conf->set("enable.auto.commit", "true", errstr);
    conf->set("auto.commit.interval.ms", "5000", errstr);

    consumer_.reset(RdKafka::KafkaConsumer::create(conf.get(), errstr));
    if (!consumer_) {
        std::cerr << "[Kafka] Failed to create consumer: " << errstr << std::endl;
        return;
    }

    RdKafka::ErrorCode err = consumer_->subscribe(topics);
    if (err != RdKafka::ERR_NO_ERROR) {
        std::cerr << "[Kafka] Subscribe failed: " << RdKafka::err2str(err) << std::endl;
    }
}

KafkaConsumer::~KafkaConsumer() {
    stop();
    if (consumer_) {
        consumer_->close();
    }
}

void KafkaConsumer::start(MessageHandler handler) {
    running_ = true;
    thread_ = std::thread([this, handler]() {
        std::cout << "[Kafka] Consumer started." << std::endl;
        while (running_) {
            auto msg = std::unique_ptr<RdKafka::Message>(consumer_->consume(1000));
            if (!msg) continue;

            switch (msg->err()) {
                case RdKafka::ERR__TIMED_OUT:
                    break;
                case RdKafka::ERR_NO_ERROR: {
                    KafkaMessage km;
                    km.topic = msg->topic_name();
                    km.key = msg->key() ? *msg->key() : "";
                    km.value = std::string(static_cast<const char*>(msg->payload()), msg->len());
                    km.offset = msg->offset();
                    km.partition = msg->partition();
                    handler(km);
                    break;
                }
                default:
                    std::cerr << "[Kafka] Error: " << msg->errstr() << std::endl;
                    break;
            }
        }
        std::cout << "[Kafka] Consumer stopped." << std::endl;
    });
}

void KafkaConsumer::stop() {
    running_ = false;
    if (thread_.joinable()) thread_.join();
}
