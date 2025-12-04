#include <iostream>
#include <string>
#include <charconv>
#include <limits>
#include <stdexcept>
#include <cstdint>
#include "read_input.h"

static inline bool will_add_overflow_int64(std::int64_t a, std::int64_t b)
{
	if (b > 0) return a > std::numeric_limits<std::int64_t>::max() - b;
	return a < std::numeric_limits<std::int64_t>::min() - b;
}

std::int64_t sum_invalid_id(const std::pair<int64_t, int64_t>& range,int part)
{
	auto is_valid_part1 = [](std::int64_t val) -> bool {
		// Effiziente Lösung mit std::to_chars:
		char buf[32];
		auto res = std::to_chars(buf, buf + sizeof(buf), val);
		if (res.ec != std::errc{}) {
			// Konvertierung fehlgeschlagen
			return false;
		}
		std::string id(buf, res.ptr); // Länge aus res.ptr ableiten
		if (id.size() % 2 != 0)
			return true;
		std::string_view first_half(id.begin(), id.begin() + id.size() / 2);
		std::string_view second_half(id.begin() + id.size() / 2, id.end());
		return first_half != second_half;
	};

	auto is_valid_part2 = [](std::int64_t val) -> bool {
		char buf[32];
		auto res = std::to_chars(buf, buf + sizeof(buf), val);
		if (res.ec != std::errc{}) return false;
		std::string_view id(buf, static_cast<std::size_t>(res.ptr - buf));

		for (std::size_t curr_period = 1; curr_period <= id.size() / 2; ++curr_period) {
			if (id.size() % curr_period != 0)
				continue;
			std::string_view pattern = id.substr(0, curr_period);
			bool all_equal = true;
			for (std::size_t i = curr_period; i < id.size(); i += curr_period) {
				if (id.substr(i, curr_period) != pattern) 
				{ 
					all_equal = false; 
					break; 
				}
			}
			if (all_equal) 
				return false; // gesamte ID ist Wiederholung -> nicht valide
		}
		return true; // keine komplette Wiederholung gefunden -> valide
	};

	auto is_valid_part2_b = [](std::int64_t val) -> bool {
		char buf[32];
		auto res = std::to_chars(buf, buf + sizeof(buf), val);
		if (res.ec != std::errc{}) return false;
		std::string_view id(buf, static_cast<std::size_t>(res.ptr - buf));
		const std::size_t n = id.size();
		if (n == 0) return false;

		// KMP-Präfixfunktion (pi[i] = Länge des längsten echten Präfixes, das auch Suffix von id[0..i] ist)
		std::vector<std::size_t> pi(n);
		for (std::size_t i = 1; i < n; ++i) {
			std::size_t j = pi[i - 1];
			while (j > 0 && id[i] != id[j]) 
				j = pi[j - 1];
			if (id[i] == id[j]) 
				++j;
			pi[i] = j;
		}

		std::size_t period = n - pi[n - 1];
		// Wenn period < n und n % period == 0, ist die ganze Zeichenkette Wiederholung eines Musters
		if (period < n && (n % period) == 0) return false;
		return true;
		};


	bool (*is_valid)(int64_t) = nullptr;
	if (part == 1)
		is_valid = is_valid_part1;
	else
		is_valid = is_valid_part2_b;

	std::int64_t sum = 0;
	for (int64_t id = range.first; id <= range.second; ++id) {
		if (!is_valid(id)) {
			if (will_add_overflow_int64(sum, id)) {
				throw std::overflow_error("Overflow beim Addieren der id in sum_invalid_id");
			}
			sum += id;
		}
	}
	return sum;
}


int main(){
	//auto ranges = read_pairs("C:/source_code/advent_of_code_2025/day2/input/example.txt");
	auto ranges = read_pairs("C:/source_code/advent_of_code_2025/day2/input/input.txt");

	try {
		std::int64_t sum = 0;
		for (const auto& range : ranges) {
			auto part = sum_invalid_id(range,2);
			if (will_add_overflow_int64(sum, part)) {
				throw std::overflow_error("Overflow beim Aggregieren der Teilsummen in main");
			}
			sum += part;
		}
		std::cout << "Die Summe der nicht validen ids ist: " << sum;
	} catch (const std::overflow_error& ex) {
		std::cerr << "Fehler: " << ex.what() << '\n';
		return 1;
	}

	// std::cout << count_invalid_id({100000, 100100}) << '\n';
	return 0;
}