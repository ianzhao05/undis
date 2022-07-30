#pragma once

#include <cstdint>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

#include "storevalue.h"

class Serializer {
    using Path = std::filesystem::path;

    template <typename Hash, typename KeyEqual, typename Allocator>
    using Map =
        std::unordered_map<std::string, StoreValue, Hash, KeyEqual, Allocator>;

  public:
    explicit Serializer(Path filename) : dbfile_{std::move(filename)} {}

    template <typename Hash, typename KeyEqual, typename Allocator>
    Serializer &operator<<(const Map<Hash, KeyEqual, Allocator> &);

    template <typename Hash, typename KeyEqual, typename Allocator>
    Serializer &operator>>(Map<Hash, KeyEqual, Allocator> &);

  private:
    Path dbfile_;
};

template <typename Hash, typename KeyEqual, typename Allocator>
Serializer &Serializer::operator<<(const Map<Hash, KeyEqual, Allocator> &mp) {
    using std::uint32_t;

    std::ofstream ofs{dbfile_,
                      std::ios::out | std::ios::binary | std::ios::trunc};
    if (!ofs) {
        return *this;
    }

    ofs.write("UNDS", 4);

    uint32_t size = 0;
    ofs.seekp(sizeof size + 4);

    auto now = std::time(nullptr);
    for (const auto &[k, v] : mp) {
        if (v.exp_time <= now) {
            continue;
        }
        ++size;

        ofs.write(reinterpret_cast<const char *>(&v.exp_time),
                  sizeof v.exp_time);

        const auto &str = v.str_val;
        auto klen = static_cast<uint32_t>(k.size()),
             vlen = static_cast<uint32_t>(str.size());

        ofs.write(reinterpret_cast<const char *>(&klen), sizeof klen);
        ofs.write(k.data(), klen);

        ofs.write(reinterpret_cast<const char *>(&vlen), sizeof vlen);
        ofs.write(str.data(), vlen);

        ofs.write(reinterpret_cast<const char *>(&v.flags), sizeof v.flags);
    }

    ofs.seekp(4);
    ofs.write(reinterpret_cast<const char *>(&size), sizeof size);

    return *this;
}

template <typename Hash, typename KeyEqual, typename Allocator>
Serializer &Serializer::operator>>(Map<Hash, KeyEqual, Allocator> &mp) {
    using std::uint32_t;

    std::ifstream ifs{dbfile_, std::ios::in | std::ios::binary};
    if (!ifs) {
        return *this;
    }

    char h[4];
    ifs.read(h, 4);
    if (!(h[0] == 'U' && h[1] == 'N' && h[2] == 'D' && h[3] == 'S')) {
        return *this;
    }

    uint32_t size;
    ifs.read(reinterpret_cast<char *>(&size), sizeof size);

    mp.clear();
    mp.reserve(size);

    auto now = std::time(nullptr);
    while (size-- > 0) {
        uint32_t klen, vlen, flags, exp_time;

        ifs.read(reinterpret_cast<char *>(&exp_time), sizeof exp_time);
        if (exp_time <= now) {
            ifs.read(reinterpret_cast<char *>(&klen), sizeof klen);
            ifs.seekg(klen, std::ios::cur);
            ifs.read(reinterpret_cast<char *>(&vlen), sizeof vlen);
            ifs.seekg(vlen + 4, std::ios::cur);
            continue;
        }

        ifs.read(reinterpret_cast<char *>(&klen), sizeof klen);
        std::string k(klen, '0');
        ifs.read(k.data(), klen);

        ifs.read(reinterpret_cast<char *>(&vlen), sizeof vlen);
        std::string v(vlen, '0');
        ifs.read(v.data(), vlen);

        ifs.read(reinterpret_cast<char *>(&flags), sizeof flags);

        mp.emplace(std::move(k), StoreValue{std::move(v), flags, exp_time});
    }

    return *this;
}
