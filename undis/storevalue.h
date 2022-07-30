#pragma once

#include <cstdint>
#include <ctime>
#include <type_traits>

struct StoreValue {
    std::string str_val;
    std::uint32_t flags;
    std::uint32_t exp_time;

    template <typename T>
        requires std::is_convertible_v<T, std::string>
    StoreValue(T &&str_val, std::uint32_t flags, int exp)
        : str_val{str_val}, flags{flags}, exp_time{} {
        if (exp < 0) {
            return;
        }
        if (exp == 0) {
            exp_time = -1;
        } else if (exp <= 60 * 60 * 24 * 30) {
            exp_time = std::time(nullptr) + exp;
        } else {
            exp_time = exp;
        }
    }

    // TODO: Fix awful overload design
    template <typename T>
        requires std::is_convertible_v<T, std::string>
    StoreValue(T &&str_val, std::uint32_t flags, std::uint32_t exp)
        : str_val{str_val}, flags{flags}, exp_time{exp} {}

    friend auto operator<=>(const StoreValue &lhs,
                            const StoreValue &rhs) = default;
};
