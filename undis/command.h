#pragma once

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
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

    template <typename... Args>
    std::string execute(KVStore &store, Args &&...args);
    std::string execute(KVStore &store);

  private:
    void parse();

    std::string s_;
    command_types::CommandVariant command_;
};

template <typename... Args>
std::string Command::execute(KVStore &store, Args &&...args) {
    using namespace command_types;

    if (auto *c = std::get_if<Storage>(&command_)) {
        bool stored = false;
        auto &&data = std::get<0>(std::make_tuple(args...));
        switch (c->type) {
        case StorageType::set:
            store.set(std::move(c->key), std::forward<Args>(args)...);
            stored = true;
            break;

        case StorageType::add:
            stored = store.add(std::move(c->key), std::forward<Args>(args)...);
            break;

        case StorageType::replace:
            stored = store.replace(c->key, std::forward<Args>(args)...);
            break;

        case StorageType::append:
            stored = store.append(c->key, std::forward<decltype(data)>(data));
            break;

        case StorageType::prepend:
            stored = store.prepend(c->key, std::forward<decltype(data)>(data));
            break;
        }

        command_ = {};
        return stored ? "STORED\r\n" : "NOT_STORED\r\n";
    }

    throw std::invalid_argument{"Invalid command"};
}
