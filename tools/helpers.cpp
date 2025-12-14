#include "helpers.h"

#include <limits>
#include <stdexcept>

namespace tools {

unsigned long long pow2_u64(unsigned int exponent)
{
    constexpr unsigned int maxBits = std::numeric_limits<unsigned long long>::digits;
    if (exponent >= maxBits) {
        throw std::overflow_error("2^N exceeds the range of unsigned long long");
    }

    return 1ULL << exponent;
}

} // namespace tools
