#pragma once

#include <cstdint>
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

    uint32_t size = static_cast<uint32_t>(mp.size()),
             buckets = static_cast<uint32_t>(mp.bucket_count());
    ofs.write(reinterpret_cast<char *>(&size), sizeof size);
    ofs.write(reinterpret_cast<char *>(&buckets), sizeof buckets);

    auto it = mp.begin();
    while (size-- > 0) {
        const auto &[k, v] = *it++;
        const auto &str = v.str_val;
        uint32_t klen = static_cast<uint32_t>(k.size()),
                 vlen = static_cast<uint32_t>(str.size()), flags = v.flags;
        ofs.write(reinterpret_cast<char *>(&klen), sizeof klen);
        ofs.write(k.data(), klen);
        ofs.write(reinterpret_cast<char *>(&vlen), sizeof vlen);
        ofs.write(str.data(), vlen);
        ofs.write(reinterpret_cast<char *>(&flags), sizeof flags);
    }

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

    uint32_t size, buckets;
    ifs.read(reinterpret_cast<char *>(&size), sizeof size);
    ifs.read(reinterpret_cast<char *>(&buckets), sizeof buckets);

    mp.clear();
    mp.rehash(buckets);

    while (size-- > 0) {
        uint32_t klen, vlen, flags;

        ifs.read(reinterpret_cast<char *>(&klen), sizeof klen);
        std::string k(klen, '0');
        ifs.read(k.data(), klen);

        ifs.read(reinterpret_cast<char *>(&vlen), sizeof vlen);
        std::string v(vlen, '0');
        ifs.read(v.data(), vlen);

        ifs.read(reinterpret_cast<char *>(&flags), sizeof flags);

        mp.emplace(std::move(k), StoreValue{std::move(v), flags});
    }

    return *this;
}
