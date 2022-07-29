#pragma once

#include <random>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../undis/storevalue.h"

inline std::string random_string(std::string::size_type length) {
    constexpr std::string_view chrs = "0123456789"
                                      "abcdefghijklmnopqrstuvwxyz"
                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    thread_local static std::mt19937 rng{std::random_device{}()};
    thread_local static std::uniform_int_distribution<std::string::size_type>
        dist{0, chrs.size() - 1};

    std::string s(length, '\0');
    std::generate_n(s.begin(), length, [&]() { return chrs[dist(rng)]; });

    return s;
}

inline std::unordered_map<std::string, StoreValue>
map_factory(std::string::size_type size) {
    std::unordered_map<std::string, StoreValue> m;
    for (std::string::size_type i = 0; i < size; ++i) {
        m.emplace(random_string(size), StoreValue{random_string(size), 0});
    }
    return m;
}
