
#pragma once

#include <iostream>
#include <map>
#include <string>
#include <optional>
#include <vector>
#include <span>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <algorithm>
#include <thread>

template <typename Clock = std::chrono::system_clock>
class KVStorage {
public:
    
    explicit KVStorage(std::span<std::tuple<std::string /*key*/, std::string /*value*/, uint32_t /*ttl*/>> entries, Clock clock = Clock());

    
    void set(std::string key, std::string value, uint32_t ttl);

    
    bool remove(std::string_view key);

    
    std::optional<std::string> get(std::string_view key) const;

    
    std::vector<std::pair<std::string, std::string>> getManySorted(std::string_view key, uint32_t count) const;

    
    std::optional<std::pair<std::string, std::string>> removeOneExpiredEntry();

private:
    void remove_(std::string_view key);
    struct item {
        std::string value;
        typename Clock::time_point time_death; // Use typename for dependent type
    };
    std::map<std::string, item> storage;
    mutable std::shared_mutex shared_mtx;
    mutable std::mutex mtx;
};
