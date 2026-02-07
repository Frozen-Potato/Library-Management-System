#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <sstream>
#include <vector>
#include <chrono>
#include <cmath>

// Prometheus-compatible metrics registry

class Counter {
public:
    explicit Counter(const std::string& name, const std::string& help)
        : name_(name), help_(help), value_(0) {}
    void inc(double v = 1.0) { value_ += v; }
    double value() const { return value_.load(); }
    const std::string& name() const { return name_; }
    const std::string& help() const { return help_; }
private:
    std::string name_;
    std::string help_;
    std::atomic<double> value_;
};

class Gauge {
public:
    explicit Gauge(const std::string& name, const std::string& help)
        : name_(name), help_(help), value_(0) {}
    void set(double v) { value_ = v; }
    void inc(double v = 1.0) { value_ += v; }
    void dec(double v = 1.0) { value_ -= v; }
    double value() const { return value_.load(); }
    const std::string& name() const { return name_; }
    const std::string& help() const { return help_; }
private:
    std::string name_;
    std::string help_;
    std::atomic<double> value_;
};

class Histogram {
public:
    Histogram(const std::string& name, const std::string& help,
              std::vector<double> buckets = {0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0, 10.0})
        : name_(name), help_(help), buckets_(std::move(buckets)),
          bucketCounts_(buckets_.size() + 1, 0), sum_(0), count_(0) {}

    void observe(double value) {
        std::lock_guard<std::mutex> lock(mtx_);
        sum_ += value;
        count_++;
        for (size_t i = 0; i < buckets_.size(); ++i) {
            if (value <= buckets_[i]) {
                bucketCounts_[i]++;
            }
        }
        bucketCounts_[buckets_.size()]++; // +Inf bucket
    }

    std::string serialize() const {
        std::lock_guard<std::mutex> lock(mtx_);
        std::ostringstream os;
        os << "# HELP " << name_ << " " << help_ << "\n";
        os << "# TYPE " << name_ << " histogram\n";
        uint64_t cumulative = 0;
        for (size_t i = 0; i < buckets_.size(); ++i) {
            cumulative += bucketCounts_[i];
            os << name_ << "_bucket{le=\"" << buckets_[i] << "\"} " << cumulative << "\n";
        }
        os << name_ << "_bucket{le=\"+Inf\"} " << count_ << "\n";
        os << name_ << "_sum " << sum_ << "\n";
        os << name_ << "_count " << count_ << "\n";
        return os.str();
    }

    const std::string& name() const { return name_; }
    const std::string& help() const { return help_; }

private:
    std::string name_;
    std::string help_;
    std::vector<double> buckets_;
    mutable std::mutex mtx_;
    std::vector<uint64_t> bucketCounts_;
    double sum_;
    uint64_t count_;
};

class ScopedTimer {
public:
    ScopedTimer(Histogram& h) : histogram_(h), start_(std::chrono::steady_clock::now()) {}
    ~ScopedTimer() {
        auto elapsed = std::chrono::steady_clock::now() - start_;
        double seconds = std::chrono::duration<double>(elapsed).count();
        histogram_.observe(seconds);
    }
private:
    Histogram& histogram_;
    std::chrono::steady_clock::time_point start_;
};

class MetricsRegistry {
public:
    static MetricsRegistry& instance() {
        static MetricsRegistry reg;
        return reg;
    }

    Counter& counter(const std::string& name, const std::string& help = "") {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = counters_.find(name);
        if (it != counters_.end()) return *it->second;
        auto c = std::make_unique<Counter>(name, help);
        auto& ref = *c;
        counters_[name] = std::move(c);
        return ref;
    }

    Gauge& gauge(const std::string& name, const std::string& help = "") {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = gauges_.find(name);
        if (it != gauges_.end()) return *it->second;
        auto g = std::make_unique<Gauge>(name, help);
        auto& ref = *g;
        gauges_[name] = std::move(g);
        return ref;
    }

    Histogram& histogram(const std::string& name, const std::string& help = "") {
        std::lock_guard<std::mutex> lock(mtx_);
        auto it = histograms_.find(name);
        if (it != histograms_.end()) return *it->second;
        auto h = std::make_unique<Histogram>(name, help);
        auto& ref = *h;
        histograms_[name] = std::move(h);
        return ref;
    }

    std::string serialize() const {
        std::lock_guard<std::mutex> lock(mtx_);
        std::ostringstream os;
        for (auto& [name, c] : counters_) {
            os << "# HELP " << c->name() << " " << c->help() << "\n";
            os << "# TYPE " << c->name() << " counter\n";
            os << c->name() << " " << c->value() << "\n";
        }
        for (auto& [name, g] : gauges_) {
            os << "# HELP " << g->name() << " " << g->help() << "\n";
            os << "# TYPE " << g->name() << " gauge\n";
            os << g->name() << " " << g->value() << "\n";
        }
        for (auto& [name, h] : histograms_) {
            os << h->serialize();
        }
        return os.str();
    }

private:
    MetricsRegistry() = default;
    mutable std::mutex mtx_;
    std::unordered_map<std::string, std::unique_ptr<Counter>> counters_;
    std::unordered_map<std::string, std::unique_ptr<Gauge>> gauges_;
    std::unordered_map<std::string, std::unique_ptr<Histogram>> histograms_;
};
