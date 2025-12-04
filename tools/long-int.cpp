#include "long-int.h"

namespace tools {

// Schoolbook multiplication producing lower 256 bits (mod 2^256)
LongInt256 operator*(const LongInt256& a, const LongInt256& b) noexcept {
    LongInt256 r;

#if defined(_MSC_VER)
    // Use intrinsics: _umul128 for 64x64->128 and _addcarry_u64 for carries.
    uint64_t res[LongInt256::nwords * 2] = {};
    for (size_t i = 0; i < LongInt256::nwords; ++i) {
        unsigned long long carry = 0;
        for (size_t j = 0; j < LongInt256::nwords; ++j) {
            size_t k = i + j;
            unsigned long long hi;
            unsigned long long lo = _umul128(a.w_[i], b.w_[j], &hi);

            // add lo to res[k]
            unsigned long long sum1;
            unsigned char c1 = _addcarry_u64(0, res[k], lo, &sum1);
            // add previous carry to sum1
            unsigned char c1b = _addcarry_u64(c1, sum1, carry, &res[k]);

            // add hi to res[k+1]
            unsigned long long sum2;
            unsigned char c2 = _addcarry_u64(0, res[k+1], hi, &sum2);
            // add carry from low additions to sum2
            unsigned char c2b = _addcarry_u64(c2, sum2, c1b, &res[k+1]);

            carry = c2b;
        }
        // write carry into res[i + nwords]
        size_t pos = i + LongInt256::nwords;
        unsigned char c = _addcarry_u64(0, res[pos], carry, &res[pos]);
        ++pos;
        // propagate any carry further
        while (c && pos < LongInt256::nwords * 2) {
            c = _addcarry_u64(c, res[pos], 0ULL, &res[pos]);
            ++pos;
        }
    }
    // copy lower nwords into result
    for (size_t i = 0; i < LongInt256::nwords; ++i) r.w_[i] = res[i];

#else
    // Use unsigned __int128 for simplicity on compilers that support it
    unsigned __int128 acc[LongInt256::nwords * 2]{};
    for (size_t i = 0; i < LongInt256::nwords; ++i) {
        for (size_t j = 0; j < LongInt256::nwords; ++j) {
            acc[i + j] += (unsigned __int128)a.w_[i] * b.w_[j];
        }
    }
    // propagate carries
    unsigned __int128 carry = 0;
    for (size_t k = 0; k < LongInt256::nwords; ++k) {
        unsigned __int128 s = acc[k] + carry;
        r.w_[k] = (LongInt256::word_t)s;
        carry = s >> 64;
    }
#endif
    return r;
}

} // namespace tools