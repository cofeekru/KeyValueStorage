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
    // конструктор работает за O(n*log(n)), где n - количество записей
    explicit KVStorage(std::span<std::tuple<std::string /*key*/, std::string /*value*/, uint32_t /*ttl*/>> entries, Clock clock = Clock()) {
        for (uint32_t i = 0; i < entries.size(); i++) {
            std::string key = std::get<0>(entries[i]);
            std::string value = std::get<1>(entries[i]);
            uint32_t ttl = std::get<2>(entries[i]);

            auto time_death = (ttl == 0) ? Clock::time_point::max() : Clock::now() + std::chrono::seconds(ttl);
            storage[key] = {value, time_death};
        }
    }

    // set работает за O(log(n)), где n - размер storage
    void set(std::string key, std::string value, uint32_t ttl) {
        std::scoped_lock lock(mtx);
        auto time_death = (ttl == 0) ? Clock::time_point::max() : Clock::now() + std::chrono::seconds(ttl);
        storage[key] = {value, time_death};
    }

    // remove работает за O(log(n)), где n - размер storage
    bool remove(std::string_view key) {
        return remove_(key);
    }

    // get работает за O(log(n)), где n - размер storage
    std::optional<std::string> get(std::string_view key) const {
        std::shared_lock lock(shared_mtx);
        auto iter = storage.find(std::string(key));
        if (iter != storage.end() && iter->second.time_death > Clock::now()) {
            return std::move(std::make_optional<std::string>(iter->second.value));
        } else {
            return std::nullopt;
        }
        
    }

    // get работает за O(n), где n - размер storage
    std::vector<std::pair<std::string, std::string>> getManySorted(std::string_view key, uint32_t count) const {
        std::shared_lock lock(shared_mtx);

        std::vector<std::pair<std::string, std::string>> result;
        auto iter_value = storage.upper_bound(std::string(key));

        while (result.size() < count && iter_value != storage.end()) {
            if (iter_value->second.time_death > Clock::now()) {
                result.emplace_back(iter_value->first, iter_value->second.value);
            }
            iter_value++;
        }
        return std::move(result);
    }

    // removeOneExpiredEntry работает за O(n), где n - размер storage
    std::optional<std::pair<std::string, std::string>> removeOneExpiredEntry() {
        std::scoped_lock lock(mtx);
        for (auto iter = storage.begin(); iter != storage.end(); iter++) {
            if (iter->second.time_death <= Clock::now()) {
                std::string key = iter->first;
                auto result = std::make_optional<std::pair<std::string, std::string>>(std::make_pair(key, iter->second.value));
                this->remove_(key);
                return std::move(result);
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
