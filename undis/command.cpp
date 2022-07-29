#include "command.h"

Command::Command(std::string command) : s_{std::move(command)} { parse(); }

CommandStatus Command::set_command(std::string command) {
    s_ = std::move(command);
    parse();
    return status();
}

CommandStatus Command::status() const {
    using namespace command_types;

    if (std::holds_alternative<Storage>(command_)) {
        return data_required;
    }
    return std::holds_alternative<std::monostate>(command_) ? invalid_command
                                                            : valid_command;
}

std::string Command::execute(KVStore &store) {
    using namespace command_types;

    if (const auto *c = std::get_if<Retrieval>(&command_)) {
        std::string reply;

        for (const std::string &key : c->keys) {
            if (auto val = store.get(key); val.has_value()) {
                reply.append("VALUE ")
                    .append(key)
                    .append(" ")
                    .append(std::to_string(val->str_val.size()))
                    .append("\r\n")
                    .append(val->str_val)
                    .append("\r\n");
            }
        }
        reply.append("END\r\n");

        command_ = {};
        return reply;
    }

    if (const auto *c = std::get_if<Deletion>(&command_)) {
        bool deleted = store.del(c->key);

        command_ = {};
        return deleted ? "DELETED\r\n" : "NOT_FOUND\r\n";
    }

    throw std::invalid_argument{"Invalid command"};
}

void Command::parse() {
    using namespace command_types;

    if (s_.empty()) {
        return;
    }

    std::istringstream is{s_};

    std::string command;
    is >> command;

    auto it = storage_type_map.find(command);
    if (it != storage_type_map.end()) {
        std::string key;
        is >> key;

        command_.emplace<Storage>(Storage{it->second, key});
    } else if (command == "get") {
        std::vector<std::string> keys;
        std::copy(std::istream_iterator<std::string>{is},
                  std::istream_iterator<std::string>{},
                  std::back_inserter(keys));

        command_.emplace<Retrieval>(Retrieval{keys});
    } else if (command == "delete") {
        std::string key;
        is >> key;

        command_.emplace<Deletion>(Deletion{key});
    }
}
