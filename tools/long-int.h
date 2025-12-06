#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <ostream>
#include <algorithm>
#include <compare>
#include <iostream>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace tools {


    class LongInt256 {
    public:
        using word_t = uint64_t;
        static constexpr size_t nwords = 4;

    private:
        std::array<word_t, nwords> w_{}; // least-significant word first

    public:
        // Constructors
		constexpr LongInt256() noexcept = default;
		template<std::integral Int>
		LongInt256(Int v) noexcept {
			if constexpr (std::is_signed_v<Int>) {
				if (v >= 0) {
					w_ = { static_cast<uint64_t>(v), 0, 0, 0 };
				}
				else {
					// erst als positive Magnitude setzen, dann Zweierkomplement über 256 Bit
					w_ = { static_cast<uint64_t>(-static_cast<int64_t>(v)), 0, 0, 0 };
					*this = negated();
				}
			}
			else {
				w_ = { static_cast<uint64_t>(v), 0, 0, 0 };
			}
		}

        static LongInt256 from_words(const std::array<word_t, nwords>& words) noexcept {
            LongInt256 x;
            x.w_ = words;
            return x;
        }

        // Access raw words
        constexpr const std::array<word_t, nwords>& words() const noexcept { return w_; }

        // Addition (inline)
        LongInt256& operator+=(const LongInt256& other) noexcept {
#if defined(_MSC_VER)
            unsigned char carry = 0;
            for (size_t i = 0; i < nwords; ++i) {
                unsigned long long tmp;
                carry = _addcarry_u64(carry, w_[i], other.w_[i], &tmp);
                // addcarry does not include previous carry when passed as 0/1 accordingly
                w_[i] = tmp;
            }
            // ignore final carry (truncation mod 2^256)
#else
            unsigned __int128 carry = 0;
            for (size_t i = 0; i < nwords; ++i) {
                unsigned __int128 s = (unsigned __int128)w_[i] + other.w_[i] + carry;
                w_[i] = (word_t)s;
                carry = s >> 64;
            }
#endif
            return *this;
        }

        // Multiply by 64-bit unsigned (inline)
        LongInt256& operator*=(uint64_t rhs) noexcept {
#if defined(_MSC_VER)
            unsigned long long carry = 0;
            for (size_t i = 0; i < nwords; ++i) {
                unsigned long long low;
                unsigned long long high;
                low = _umul128(w_[i], rhs, &high);
                unsigned char c = _addcarry_u64(0, (unsigned long long)low, (unsigned long long)carry, &low);
                carry = high + c; // high plus carry from low addition
                w_[i] = low;
            }
#else
            unsigned __int128 carry = 0;
            for (size_t i = 0; i < nwords; ++i) {
                unsigned __int128 p = (unsigned __int128)w_[i] * rhs + carry;
                w_[i] = (word_t)p;
                carry = p >> 64;
            }
#endif
            return *this;
        }

        // Multiply by signed int (handles negative by two's complement result)
        LongInt256 operator*(int64_t rhs) const noexcept {
            bool neg = rhs < 0;
            uint64_t urhs = static_cast<uint64_t>(neg ? -static_cast<int64_t>(rhs) : rhs);
            LongInt256 r = *this;
            r *= urhs;
            if (neg) r = r.negated();
            return r;
        }

        // Two's complement negation
        LongInt256 negated() const noexcept {
            LongInt256 r;
            // bitwise not
            for (size_t i = 0; i < nwords; ++i) r.w_[i] = ~w_[i];
            // add one
#if defined(_MSC_VER)
            unsigned char carry = 0;
            unsigned long long tmp;
            carry = _addcarry_u64(0, r.w_[0], 1ULL, &tmp);
            r.w_[0] = tmp;
            for (size_t i = 1; i < nwords && carry; ++i) {
                carry = _addcarry_u64(carry, r.w_[i], 0ULL, &tmp);
                r.w_[i] = tmp;
            }
#else
            unsigned __int128 carry = 1;
            for (size_t i = 0; i < nwords; ++i) {
                unsigned __int128 s = (unsigned __int128)r.w_[i] + carry;
                r.w_[i] = (word_t)s;
                carry = s >> 64;
            }
#endif
            return r;
        }

        // Convenience conversion to hex string for debugging (inline)
        std::string to_hex() const {
            std::ostringstream ss;
            ss << std::hex << std::setfill('0');
            // print most-significant word first
            for (size_t i = nwords; i-- > 0;) {
                ss << std::setw(16) << w_[i];
            }
            return ss.str();
        }

        // Convert to decimal string (signed two's-complement interpretation)
        std::string to_dec() const {
            // Check for zero
            bool is_zero = true;
            for (size_t i = 0; i < nwords; ++i) if (w_[i] != 0) { is_zero = false; break; }
            if (is_zero) return "0";

            // Determine sign (two's complement signed interpretation)
            bool negative = (w_[nwords - 1] & (1ULL << 63)) != 0;
            LongInt256 tmp = *this;
            if (negative) tmp = tmp.negated(); // absolute value in tmp

            std::array<word_t, nwords> a = tmp.w_;
            std::string digits;
            digits.reserve(80);

            // Repeatedly divide by 10, collecting remainders
            while (true) {
                // check if a == 0
                bool allzero = true;
                for (size_t i = 0; i < nwords; ++i) { if (a[i] != 0) { allzero = false; break; } }
                if (allzero) break;

                uint64_t rem = 0;
                for (size_t idx = nwords; idx-- > 0;) {
#if defined(_MSC_VER)
                    // divide 128-bit value (rem<<64 | a[idx]) by 10 using MSVC intrinsic
                    uint64_t q = _udiv128(rem, a[idx], 10ULL, &rem);
                    a[idx] = q;
#else
                    unsigned __int128 cur = ((unsigned __int128)rem << 64) | a[idx];
                    uint64_t q = (uint64_t)(cur / 10);
                    rem = (uint64_t)(cur % 10);
                    a[idx] = q;
#endif
                }
                digits.push_back(char('0' + rem));
            }

            if (digits.empty()) digits = "0";
            else std::reverse(digits.begin(), digits.end());
            if (negative) digits.insert(digits.begin(), '-');
            return digits;
        }

        // Three-way comparison (signed two's-complement interpretation)
        constexpr auto operator<=>(const LongInt256& o) const noexcept -> std::strong_ordering {
            // sign bit of most-significant word
            const bool s1 = (w_[nwords - 1] & (1ULL << 63)) != 0;
            const bool s2 = (o.w_[nwords - 1] & (1ULL << 63)) != 0;

            // different signs: negative < positive
            if (s1 != s2) return s1 ? std::strong_ordering::less : std::strong_ordering::greater;

            // same sign: compare words from most-significant to least-significant
            for (size_t i = nwords; i-- > 0;) {
                if (w_[i] < o.w_[i]) return std::strong_ordering::less;
                if (w_[i] > o.w_[i]) return std::strong_ordering::greater;
            }
            return std::strong_ordering::equal;
        }

        // Equality
        bool operator==(const LongInt256& o) const noexcept { return w_ == o.w_; }
        bool operator!=(const LongInt256& o) const noexcept { return !(*this == o); }

        // Friend declarations for non-member operators
        friend LongInt256 operator*(const LongInt256& a, const LongInt256& b) noexcept; // implemented in cpp
        friend LongInt256 operator*(const LongInt256& a, uint64_t b) noexcept { LongInt256 r = a; r *= b; return r; }
        friend LongInt256 operator*(uint64_t a, const LongInt256& b) noexcept { LongInt256 r = b; r *= a; return r; }
        friend LongInt256 operator+(LongInt256 a, const LongInt256& b) noexcept { a += b; return a; }
        friend LongInt256 operator-(const LongInt256& a,const LongInt256& b) noexcept { 
            // Verwende die vorhandene Zweierkomplement-Negation von b statt LongInt256(-1) * b
            LongInt256 tmp = b.negated();
            return a + tmp;
        }
    };

	template <typename T>
	concept BuiltinInteger = std::is_integral_v<T> && !std::is_same_v<T, bool>;

	template <typename T>
	inline T string_to_integer(std::string_view s)
	{
		if constexpr (BuiltinInteger<T>) {
			T value{};
			const char* first = s.data();
			const char* last = s.data() + s.size();

			auto [ptr, ec] = std::from_chars(first, last, value, 10);
			if (ec != std::errc{} || ptr != last) {
				throw std::invalid_argument{ "invalid integer string" };
			}
			return value;
		}
		else {
			if (s.empty()) {
				throw std::invalid_argument{ "invalid integer string" };
			}

			bool negative = false;
			if (s.front() == '+' || s.front() == '-') {
				negative = (s.front() == '-');
				s.remove_prefix(1);
				if (s.empty()) {
					throw std::invalid_argument{ "invalid integer string" };
				}
			}

			T value = T(0);
			const T ten = T(10);

			for (char c : s) {
				if (c < '0' || c > '9') {
					throw std::invalid_argument{ "invalid integer string" };
				}
				int digit = c - '0';
				value = value * ten + T(digit);
			}

			if (negative) {
				// Verwende Multiplikation mit -1, um nur '*' und Konstruktion aus int zu benötigen.
				value = value * T(-1);
			}

			return value;
		}
	}

    // Output operator: print decimal representation
    inline std::ostream& operator<<(std::ostream& os, const LongInt256& v) {
        return os << v.to_dec();
    }
} // namespace tools