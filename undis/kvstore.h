#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "serializer.h"
#include "storevalue.h"

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

    template <typename K, typename V>
    void set(K &&key, V &&value, std::uint32_t flags);

    template <typename K, typename V>
    bool add(K &&key, V &&value, std::uint32_t flags);

    template <typename V>
    bool replace(std::string_view key, V &&value, std::uint32_t flags);

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

template <typename K, typename V>
void KVStore::set(K &&key, V &&value, std::uint32_t flags) {
    std::scoped_lock lk{mtx_};
    map_[std::forward<K>(key)] = {std::forward<V>(value), flags};
}

template <typename K, typename V>
bool KVStore::add(K &&key, V &&value, std::uint32_t flags) {
    std::scoped_lock lk{mtx_};
    return map_
        .try_emplace(std::forward<K>(key),
                     StoreValue{std::forward<V>(value), flags})
        .second;
}

template <typename V>
bool KVStore::replace(std::string_view key, V &&value, std::uint32_t flags) {
    std::scoped_lock lk{mtx_};
    auto it = map_.find(key);
    if (it != map_.end()) {
        it->second = {std::forward<V>(value), flags};
        return true;
    }
    return false;
}
