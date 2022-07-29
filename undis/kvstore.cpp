#include "kvstore.h"

KVStore::KVStore(std::filesystem::path filename) : ser_{std::move(filename)} {
    *ser_ >> map_;
}

KVStore::~KVStore() {
    if (ser_.has_value()) {
        *ser_ << map_;
    }
}

std::optional<StoreValue> KVStore::get(std::string_view key) const {
    std::shared_lock lk{mtx_};
    auto it = map_.find(key);
    return (it != map_.end()) ? std::optional{it->second} : std::nullopt;
}

bool KVStore::append(std::string_view key, std::string_view suffix) {
    std::scoped_lock lk{mtx_};
    auto it = map_.find(key);
    if (it != map_.end()) {
        it->second.str_val.append(suffix);
        return true;
    }
    return false;
}

bool KVStore::prepend(std::string_view key, std::string_view prefix) {
    std::scoped_lock lk{mtx_};
    auto it = map_.find(key);
    if (it != map_.end()) {
        it->second.str_val.insert(0, prefix);
        return true;
    }
    return false;
}

bool KVStore::prepend(std::string_view key, std::string &&prefix) {
    std::scoped_lock lk{mtx_};
    auto it = map_.find(key);
    if (it != map_.end()) {
        prefix.append(it->second.str_val);
        it->second.str_val = std::move(prefix);
        return true;
    }
    return false;
}

bool KVStore::del(std::string_view key) {
    std::scoped_lock lk{mtx_};
    // C++ 23
    // return map_.erase(key);
    return map_.erase(std::string{key});
}

std::size_t KVStore::size() const {
    std::scoped_lock lk{mtx_};
    return map_.size();
}
