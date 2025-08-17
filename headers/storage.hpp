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
    
    explicit KVStorage(std::span<std::tuple<std::string /*key*/, std::string /*value*/, uint32_t /*ttl*/>> entries, Clock clock = Clock()) {
        for (uint32_t i = 0; i < entries.size(); i++) {
            std::string key = std::get<0>(entries[i]);
            std::string value = std::get<1>(entries[i]);
            uint32_t ttl = std::get<2>(entries[i]);

            auto time_death = (ttl == 0) ? Clock::time_point::max() : Clock::now() + std::chrono::seconds(ttl);
            storage[key] = {value, time_death};
        }
    }

    
    void set(std::string key, std::string value, uint32_t ttl) {
        std::scoped_lock lock(mtx);
        auto time_death = (ttl == 0) ? Clock::time_point::max() : Clock::now() + std::chrono::seconds(ttl);
        storage[key] = {value, time_death};
    }


    bool remove(std::string_view key) {
        return remove_(key);
    }

    
    std::optional<std::string> get(std::string_view key) const {
        std::shared_lock lock(shared_mtx);
        auto iter = storage.find(std::string(key));
        if (iter != storage.end() && iter->second.time_death > Clock::now()) {
            auto value = std::make_optional<std::string>(iter->second.value);
            return value;
        } else {
            return std::nullopt;
        }
        
    }

    
    std::vector<std::pair<std::string, std::string>> getManySorted(std::string_view key, uint32_t count) const {
        std::shared_lock lock(shared_mtx);

        std::vector<std::pair<std::string, std::string>> result;
        auto iter_value = storage.upper_bound(std::string(key));

        while (result.size() < count && iter_value != storage.end()) {
            if (iter_value->second.time_death > Clock::now()) {
                result.push_back(std::make_pair(iter_value->first, iter_value->second.value));
            }
            iter_value++;
        }
        return result;
    }

    
    std::optional<std::pair<std::string, std::string>> removeOneExpiredEntry() {
        std::scoped_lock lock(mtx);
        for (auto iter = storage.begin(); iter != storage.end(); iter++) {
            if (iter->second.time_death <= Clock::now()) {
                std::string key = iter->first;
                auto result = std::make_optional<std::pair<std::string, std::string>>(std::make_pair(key, iter->second.value));
                this->remove_(key);
                return result;
            }
        }
        return std::nullopt;
    }

private:
    bool remove_(std::string_view key) {
        return 1 == storage.erase(std::string(key));    
    }
    struct item {
        std::string value;
        Clock::time_point time_death;
    };
    std::map<std::string, item> storage;
    mutable std::shared_mutex shared_mtx;
    mutable std::mutex mtx;
};
