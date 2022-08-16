#include "server.h"

#include "connectionhandler.h"

// Parts adapted from https://beej.us/guide/bgnet/html/

volatile static std::sig_atomic_t stop = 0;

Server::Server(unsigned port, KVStore &store)
    : store_{store}, wsaclean_{false} {
#ifdef _WIN32
    WSAData wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data)) {
        throw std::runtime_error{"WSAStartup failed."};
    }
    wsaclean_.to_clean = true;
#endif

    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> servinfo{nullptr,
                                                                &freeaddrinfo};

    addrinfo *servinfo_raw{};
    auto res = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints,
                           &servinfo_raw);
    servinfo.reset(servinfo_raw);

    // C++ 23
    // if (getaddrinfo(NULL, std::to_string(port).c_str(), &hints,
    // std::out_ptr(servinfo)); != 0) {
    if (res != 0) {
        throw std::runtime_error{"getaddrinfo failed."};
    }

    addrinfo *p;
    for (p = servinfo.get(); p != nullptr; p = p->ai_next) {
        if ((sockfd_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
            INVALID_SOCKET) {
            std::cerr << "socket error.\n";
            continue;
        }

        if (int yes = 1; setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                                    reinterpret_cast<char *>(&yes),
                                    sizeof yes) == SOCKET_ERROR) {
            throw std::runtime_error{"setsockopt failed."};
        }

        if (bind(sockfd_, p->ai_addr, p->ai_addrlen) == 0) {
            break;
        } else {
            std::cerr << "bind error\n";
            close_socket(sockfd_);
        }
    }

    servinfo.reset(nullptr);

    if (!p) {
        throw std::runtime_error{"failed to bind."};
    }

    if (listen(sockfd_, 10)) {
        throw std::runtime_error{"listen failed."};
    }

    std::cout << "Server initialized on port " << port << '\n';
}

Server::~Server() { close_socket(sockfd_); }

void Server::start() {
#ifdef _WIN32
    // TODO Windows signal handling
#else
    struct sigaction sa;
    sa.sa_handler = Server::sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
#endif

    tp_.emplace(1, 10, std::chrono::seconds(5));

    std::cout << "Waiting for connections...\n";
    while (!stop) {
        SOCKET newfd = accept(sockfd_, nullptr, nullptr);

        if (newfd != INVALID_SOCKET) {
            std::cout << "Got connection\n";
            tp_->queue_job(ConnectionHandler{store_, newfd});
        }
    }
    std::cout << "Stopping...\n";
}

void Server::close_socket(SOCKET fd) {
#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif
}

void Server::sig_handler(int s) { stop = 1; }
