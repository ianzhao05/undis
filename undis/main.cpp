#include <charconv>
#include <iostream>
#include <string_view>

#include "kvstore.h"
#include "server.h"

using namespace std::literals;

int main(int argc, char **argv) {
    std::string_view filename{"undis.db"sv};
    unsigned port = 8080;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg{argv[i]};
        if (arg == "-f") {
            if (i + 1 >= argc) {
                std::cerr << "Expected filename after -f\n";
                return 3;
            }
            filename = argv[++i];
        } else if (arg == "-p") {
            if (i + 1 >= argc) {
                std::cerr << "Expected port number after -p\n";
                return 3;
            }
            std::string_view port_str{argv[++i]};
            if (auto res = std::from_chars(
                    port_str.data(), port_str.data() + port_str.size(), port);
                res.ec != std::errc{}) {
                std::cerr << "Invalid port number: " << port_str << '\n';
                return 4;
            }
        } else {
            std::cerr << "Unknown option: " << arg << "\nUsage: " << argv[0]
                      << " [-f filename (undis.db)] [-p port (8080)]\n";
            return 2;
        }
    }

    KVStore db{filename};
    try {
        Server server{port, db};
        server.start();
        return 0;
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
