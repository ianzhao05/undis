#pragma once

#include <cstdint>

struct StoreValue {
    std::string str_val;
    std::uint32_t flags;

    friend auto operator<=>(const StoreValue &lhs,
                            const StoreValue &rhs) = default;
};
