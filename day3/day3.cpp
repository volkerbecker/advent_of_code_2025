#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <ranges>
#include <numeric>
#include <charconv>
#include <cmath>
#include "long-int.h"

struct bank
{
    std::vector<int> battery{};
};

using tools::LongInt256;

std::vector<bank> load_input(const std::string& filename)
{
	std::vector<bank> banks{};
    std::ifstream is{ filename };
    if (!is.good())
    {
        std::cerr << "Error opening file: " << filename << "\n";
        exit(1);
    }
    std::string line{};
	while (std::getline(is, line))
    {
        std::vector<int> nums(line.size());
        auto curr = nums.begin();
        for (const char c : line)
        {
			std::from_chars(&c, &c + 1, *curr);
			++curr;
        }
        banks.emplace_back(nums);	
    }
	return banks;
}

int get_max_jolt(const bank& bank)
{
	auto it_first = std::max_element(bank.battery.begin(), bank.battery.end() - 1);
	auto it_second = std::max_element(it_first + 1, bank.battery.end());
	return (*it_first) * 10 + (*it_second);
}

LongInt256 get_max_jolt(const bank& bank, int numdigits)
{
	LongInt256 result=0;
	auto it_current = bank.battery.begin();
	int64_t factor = static_cast<int64_t>(std::pow(10, numdigits - 1));
	while (factor > 0)
	{
		it_current = std::max_element(it_current, bank.battery.end() - numdigits + 1);
		result += *it_current * factor;
		it_current++;
		factor /= 10;
		numdigits--;
	}
	return result;
}

int main()
{
	//auto banks = load_input("C:/source_code/advent_of_code_2025/day3/input/example.txt");
	auto banks = load_input("C:/source_code/advent_of_code_2025/day3/input/input.txt");

	// ranges-basierte Transformation: erzeugt eine View, die für jedes `bank` get_max_jolt aufruft
	auto jolt_view = banks | std::views::transform([](const bank& b) { return get_max_jolt(b); });

	//// Werte ausgeben (optional)
	//for (int j : jolt_view)
	//	std::cout << j << '\n';

	// Summe der Werte berechnen und ausgeben
	int total = std::accumulate(std::ranges::begin(jolt_view), std::ranges::end(jolt_view), 0);
	std::cout << "Total Part 1: " << total << '\n';

	auto jolt_view_part2 = banks | std::views::transform([](const bank& b) { return get_max_jolt(b, 12); });
	for (const auto& val : jolt_view_part2)
		std::cout << val << '\n';
	LongInt256 total_part2 = std::accumulate(std::ranges::begin(jolt_view_part2), std::ranges::end(jolt_view_part2), LongInt256{0},
		[](const LongInt256& a, const LongInt256& b){ LongInt256 r = a; r += b; return r; });
	std::cout << "Total Part 2: " << total_part2 << '\n';

	return 0;
}