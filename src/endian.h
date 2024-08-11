#pragma once

#include <bit>

namespace Pss {
template <typename T>
T to_little_endian(T n) noexcept
{
    if constexpr (std::endian::native == std::endian::big) {
        return std::byteswap(n);
    }
    return n;
}

template <typename T>
T to_big_endian(T n) noexcept
{
    if constexpr (std::endian::native == std::endian::little) {
        return std::byteswap(n);
    }
    return n;
}

template <typename T>
T from_little_endian(T n) noexcept
{
    if constexpr (std::endian::native == std::endian::big) {
        return std::byteswap(n);
    }
    return n;
}

template <typename T>
T from_big_endian(T n) noexcept
{
    if constexpr (std::endian::native == std::endian::little) {
        return std::byteswap(n);
    }
    return n;
}
}  // namespace Pss