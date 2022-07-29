#pragma once

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
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
    std::string execute(KVStore &store, T &&data, std::uint32_t flags);
    std::string execute(KVStore &store);

  private:
    void parse();

    std::string s_;
    command_types::CommandVariant command_;
};

template <typename T>
std::string Command::execute(KVStore &store, T &&data, std::uint32_t flags) {
    using namespace command_types;

    if (auto *c = std::get_if<Storage>(&command_)) {
        bool stored = false;
        switch (c->type) {
        case StorageType::set:
            store.set(std::move(c->key), data, flags);
            stored = true;
            break;

        case StorageType::add:
            stored = store.add(std::move(c->key), data, flags);
            break;

        case StorageType::replace:
            stored = store.replace(c->key, data, flags);
            break;

        case StorageType::append:
            stored = store.append(c->key, data);
            break;

        case StorageType::prepend:
            stored = store.prepend(c->key, data);
            break;
        }

        command_ = {};
        return stored ? "STORED\r\n" : "NOT_STORED\r\n";
    }

    throw std::invalid_argument{"Invalid command"};
}
