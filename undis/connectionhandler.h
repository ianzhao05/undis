#pragma once

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <utility>

#include "command.h"
#include "server.h"

class KVStore;

class ConnectionHandler {
  public:
    ConnectionHandler(KVStore &store, SOCKET newfd);

    void operator()();

  private:
    KVStore &store_;
    SOCKET newfd_;

    static constexpr std::size_t BUFFER_SIZE = 1024;
    std::array<char, BUFFER_SIZE> buf_;
    std::size_t buf_pos_;

    int send_str(std::string_view s) const;
    std::string receive_line();
};
