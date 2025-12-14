#pragma once

#include <concepts>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace tools {

// Computes 2^exponent for the requested unsigned integer result type.
unsigned long long pow2_u64(unsigned int exponent);

template <std::unsigned_integral UInt>
inline UInt pow2(unsigned int exponent)
{
    if (exponent >= std::numeric_limits<UInt>::digits) {
        throw std::overflow_error("2^N exceeds the range of the requested type");
    }

    const auto value = pow2_u64(exponent);
    return static_cast<UInt>(value);
}

} // namespace tools
