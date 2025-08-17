#include "gtest/gtest.h"

#include "../headers/storage.hpp"

TEST(KVStorageTest, EmptyStorage) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> data = {};

    KVStorage<std::chrono::system_clock> storage(data);;
    ASSERT_FALSE(storage.get("key").has_value());
}

TEST(KVStorageTest, SetAndGet) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> data = {};
    KVStorage<std::chrono::system_clock> storage(data);
    storage.set("key", "value", 10);
    ASSERT_EQ(storage.get("key").value(), "value");
}

TEST(KVStorageTest, Remove) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> data = {};
    KVStorage<std::chrono::system_clock> storage(data);
    storage.set("key", "value", 10);
    ASSERT_TRUE(storage.remove("key"));
    ASSERT_FALSE(storage.get("key").has_value());
}

TEST(KVStorageTest, TTL) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> data = {};
    KVStorage<std::chrono::system_clock> storage(data);
    storage.set("key", "value", 1);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ASSERT_FALSE(storage.get("key").has_value());
}

TEST(KVStorageTest, InitialData) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> data = {
        {"key1", "value1", 10},
        {"key2", "value2", 0} 
    };
    KVStorage storage(data);
    ASSERT_EQ(storage.get("key1").value(), "value1");
    ASSERT_EQ(storage.get("key2").value(), "value2");
}

TEST(KVStorageTest, GetManySorted) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> data = {};
    KVStorage storage(data);
    storage.set("key1", "value1", 10);
    storage.set("key2", "value2", 10);
    storage.set("key3", "value3", 10);

    auto result = storage.getManySorted("key1", 2);
    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result[0].first, "key2");
    ASSERT_EQ(result[0].second, "value2");
    ASSERT_EQ(result[1].first, "key3");
    ASSERT_EQ(result[1].second, "value3");

    result = storage.getManySorted("key1", 5);
    ASSERT_EQ(result.size(), 2);

    result = storage.getManySorted("key3", 2);
    ASSERT_EQ(result.size(), 0);
}

TEST(KVStorageTest, RemoveOneExpiredEntry) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> data = {};
    KVStorage storage(data);
    storage.set("key1", "value1", 2);
    storage.set("key2", "value2", 20);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    auto removed = storage.removeOneExpiredEntry();
    ASSERT_TRUE(removed.has_value());
    ASSERT_EQ(removed->first, "key1");
    ASSERT_EQ(removed->second, "value1");

    ASSERT_FALSE(storage.get("key1").has_value());
    ASSERT_EQ(storage.get("key2").value(), "value2");
}

TEST(KVStorageTest, RemoveOneExpiredEntry_NoExpired) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> data = {};
    KVStorage storage(data);
    storage.set("key1", "value1", 30);
    storage.set("key2", "value2", 30);

    auto removed = storage.removeOneExpiredEntry();
    ASSERT_FALSE(removed.has_value());

    ASSERT_EQ(storage.get("key1").value(), "value1");
    ASSERT_EQ(storage.get("key2").value(), "value2");
}

TEST(KVStorageTest, RemoveOneExpiredEntry_MultipleExpired) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> data = {};
    KVStorage storage(data);
    storage.set("key1", "value1", 1);
    storage.set("key2", "value2", 1);
    storage.set("key3", "value3", 10);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto removed1 = storage.removeOneExpiredEntry();
    ASSERT_TRUE(removed1.has_value());

    auto removed2 = storage.removeOneExpiredEntry();
    ASSERT_TRUE(removed2.has_value());

    auto removed3 = storage.removeOneExpiredEntry();
    ASSERT_FALSE(removed3.has_value());
}

TEST(KVStorageTest, ZeroTTL) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> data = {};
    KVStorage storage(data);
    storage.set("key", "value", 0);
    ASSERT_EQ(storage.get("key").value(), "value");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ASSERT_EQ(storage.get("key").value(), "value");
}