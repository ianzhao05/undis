#include "connectionhandler.h"

ConnectionHandler::ConnectionHandler(KVStore &store, SOCKET newfd)
    : store_{store}, newfd_{newfd}, buf_{}, buf_pos_{0} {}

void ConnectionHandler::operator()() {
    using namespace std::literals;

    while (true) {
        send_str("undis > "sv);
        std::string s{receive_line()};
        if (s == "quit") {
            break;
        }

        Command c{std::move(s)};
        try {
            switch (c.status()) {
            case CommandStatus::valid_command:
                send_str(c.execute(store_));
                break;
            case CommandStatus::data_required:
                send_str(c.execute(store_, receive_line()));
                break;
            case CommandStatus::invalid_command:
                send_str("ERROR\r\n"sv);
                break;
            }
        } catch (const std::invalid_argument &err) {
            std::string err_str{"CLIENT_ERROR "sv};
            err_str += err.what();
            err_str += "\r\n"sv;
            send_str(err_str);
        }
    }

    Server::close_socket(newfd_);
}

int ConnectionHandler::send_str(std::string_view s) const {
    int sent = 0;
    int to_send = s.size();

    int n = 0;
    while (sent < s.size()) {
        n = send(newfd_, s.data() + sent, to_send, 0);
        if (n == -1) {
            break;
        }

        sent += n;
        to_send -= n;
    }

    return n;
}

std::string ConnectionHandler::receive_line() {
    constexpr std::string_view crlf = "\r\n";
    std::string line;

    while (true) {
        auto it = buf_.begin() + buf_pos_;
        int nread = recv(newfd_, &*it, BUFFER_SIZE - buf_pos_, 0);
        if (nread == 0 || nread == SOCKET_ERROR) {
            return "quit";
        }

        auto buf_end = it + nread;
        auto line_end = std::search(it, buf_end, crlf.begin(), crlf.end());

        line.append(it, line_end);

        auto dist = std::distance(line_end, buf_end);
        if (dist == 2) {
            return line;
        }

        if (dist == 0) {
            buf_pos_ = 0;
        } else {
            buf_pos_ = dist - 2;
            std::copy(line_end + 2, buf_end, buf_.begin());
        }
    }
}
