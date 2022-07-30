#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace command_types {

enum class StorageType { set, add, replace, append, prepend };

const std::unordered_map<std::string, StorageType> storage_type_map = {
    {"set", StorageType::set},         {"add", StorageType::add},
    {"replace", StorageType::replace}, {"append", StorageType::append},
    {"prepend", StorageType::prepend},
};

struct Storage {
    StorageType type;
    std::string key;
    std::uint32_t flags;
    int exp_time;
    unsigned bytes;
};

struct Retrieval {
    std::vector<std::string> keys;
};

struct Deletion {
    std::string key;
};

using CommandVariant =
    std::variant<std::monostate, Storage, Retrieval, Deletion>;

} // namespace command_types
