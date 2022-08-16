#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
using SOCKET = int;
constexpr int INVALID_SOCKET = -1;
constexpr int SOCKET_ERROR = -1;
#endif

#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "threadpool.h"

class KVStore;

class Server {
  public:
    Server(unsigned port, KVStore &store);
    ~Server();
    Server(const Server &) = delete;
    Server(Server &&) = delete;
    Server &operator=(const Server &) = delete;
    Server &operator=(Server &&) = delete;

    void start();

  private:
    struct WSACleanupWrapper {
        bool to_clean;
        ~WSACleanupWrapper() {
#ifdef _WIN32
            if (to_clean) {
                WSACleanup();
            }
#endif
        }
    };
    static void close_socket(SOCKET fd);
    static void sig_handler(int s);

    SOCKET sockfd_;

    KVStore &store_;
    std::optional<ThreadPool> tp_;

    WSACleanupWrapper wsaclean_;

    friend class ConnectionHandler;
};
