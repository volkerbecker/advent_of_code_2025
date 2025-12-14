#pragma once
#include <vector>
#include <iostream>
#include <concepts>
#include <string>
#include <string_view>
#include <stdexcept>
#include "long-int.h"

namespace tools {

    template <typename T>
    class Range {
    public:
        Range(T s, T e) : start(s), end(e) {}
        explicit Range(std::string_view str) {
            auto delimiter_pos = str.find('-');
            if (delimiter_pos == std::string_view::npos) {
                throw std::invalid_argument("Invalid range format");
            }
            auto left = std::string(str.substr(0, delimiter_pos));
            auto right = std::string(str.substr(delimiter_pos + 1));
            try {
				start = string_to_integer<T>(left);
				end = string_to_integer<T>(right);
            }
            catch (const std::exception&)
            {
                throw std::invalid_argument("Invalid number in range");
            }
        }

        bool is_in_range(T value) const {
            bool res = value >= start && value <= end;
			//std::cout << "Checking if " << value << " is in range [" << start << ", " << end << " Result: " << std::boolalpha << res  << "]\n";
            return res;
        }

        std::optional<Range<T>> union_with(const Range<T>& other) const {
            if (end < other.start || other.end < start) {
                return std::nullopt; // no overlap
            }
            T new_start = std::min(start, other.start);
            T new_end = std::max(end, other.end);
            return Range<T>(new_start, new_end);
		}
        T length() const {
            return end - start + T(1);
		}

        T get_first() const {
            return start;
		}

        T get_last() const {
            return end;
        }

    private:
        T start{};
        T end{};
    };
}