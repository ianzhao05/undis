#pragma once

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "commandtypes.h"
#include "kvstore.h"

enum class CommandStatus { valid_command, invalid_command, data_required };

class Command {
  public:
    using enum CommandStatus;

    Command() = default;
    explicit Command(std::string command);

    CommandStatus set_command(std::string command);
    CommandStatus status() const;

    template <typename T>
        requires std::convertible_to<T, std::string>
    std::string execute(KVStore &store, T &&data);
    std::string execute(KVStore &store);

  private:
    void parse();

    std::string s_;
    command_types::CommandVariant command_;
};

template <typename T>
    requires std::convertible_to<T, std::string>
std::string Command::execute(KVStore &store, T &&data) {
    using namespace command_types;

    if (auto *c = std::get_if<Storage>(&command_)) {
        bool mismatch = false;
        std::size_t size;
        if constexpr (std::is_class_v<T>) {
            size = data.size();
        } else {
            size = std::strlen(data);
        }
        if (size != c->bytes) {
            throw std::invalid_argument{
                "Mismatch between bytes and value length"};
        }

        bool stored = false;
        switch (c->type) {
        case StorageType::set:
            store.set(std::move(c->key), std::forward<T>(data), c->flags,
                      c->exp_time);
            stored = true;
            break;

        case StorageType::add:
            stored = store.add(std::move(c->key), std::forward<T>(data),
                               c->flags, c->exp_time);
            break;

        case StorageType::replace:
            stored = store.replace(c->key, std::forward<T>(data), c->flags,
                                   c->exp_time);
            break;

        case StorageType::append:
            stored = store.append(c->key, std::forward<T>(data));
            break;

        case StorageType::prepend:
            stored = store.prepend(c->key, std::forward<T>(data));
            break;
        }

        command_ = {};
        return stored ? "STORED\r\n" : "NOT_STORED\r\n";
    }

    throw std::invalid_argument{"Invalid command"};
}
