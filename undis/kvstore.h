#pragma once

#include <concepts>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <functional>
#include <limits>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "serializer.h"
#include "storevalue.h"

template <typename T>
concept Key = std::convertible_to<T, std::string>;

template <typename ...Args>
concept ValueArgs = std::constructible_from<StoreValue, Args...>;

class KVStore {
  public:
    KVStore() = default;
    explicit KVStore(std::filesystem::path filename);
    ~KVStore();
    KVStore(const KVStore &) = delete;
    KVStore &operator=(const KVStore &) = delete;

    // TODO: Implement move
    KVStore(KVStore &&) = delete;
    KVStore &operator=(KVStore &&) = delete;

    std::optional<StoreValue> get(std::string_view key) const;

    template <Key K, typename... Args>
        requires ValueArgs<Args...>
    void set(K &&key, Args &&...args);

    template <Key K, typename... Args>
        requires ValueArgs<Args...>
    bool add(K &&key, Args &&...args);

    template <typename... Args>
        requires ValueArgs<Args...>
    bool replace(std::string_view key, Args &&...args);

    bool append(std::string_view key, std::string_view suffix);

    bool prepend(std::string_view key, std::string_view prefix);
    bool prepend(std::string_view key, std::string &&prefix);

    bool del(std::string_view key);

    std::size_t size() const;

  private:
    struct StringHash {
        using is_transparent = void;
        using hash_type = std::hash<std::string_view>;
        std::size_t operator()(std::string_view txt) const {
            return hash_type{}(txt);
        }
        std::size_t operator()(const std::string &txt) const {
            return hash_type{}(txt);
        }
        std::size_t operator()(const char *txt) const {
            return hash_type{}(txt);
        }
    };

    std::unordered_map<std::string, StoreValue, StringHash, std::equal_to<>>
        map_;
    mutable std::shared_mutex mtx_;

    std::optional<Serializer> ser_;
};

template <Key K, typename... Args>
    requires ValueArgs<Args...>
void KVStore::set(K &&key, Args &&...args) {
    std::scoped_lock lk{mtx_};
    auto [it, stored] =
        map_.try_emplace(std::forward<K>(key), std::forward<Args>(args)...);
    if (!stored) {
        it->second = {std::forward<Args>(args)...};
    }
}

template <Key K, typename... Args>
    requires ValueArgs<Args...>
bool KVStore::add(K &&key, Args &&...args) {
    std::scoped_lock lk{mtx_};
    return map_.try_emplace(std::forward<K>(key), std::forward<Args>(args)...)
        .second;
}

template <typename... Args>
    requires ValueArgs<Args...>
bool KVStore::replace(std::string_view key, Args &&...args) {
    std::scoped_lock lk{mtx_};
    auto it = map_.find(key);
    if (it != map_.end()) {
        it->second = {std::forward<Args>(args)...};
        return true;
    }
    return false;
}
