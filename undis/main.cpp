#include <iostream>

#include "kvstore.h"
#include "server.h"

int main() {
    KVStore db{"undis.db"};
    try {
        Server server{8080, db};
        server.start();
        return 0;
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
