#include <iostream>
#include <string>
#include "read_input.h"
#include <cassert>
#include <string_view>
#include "day1.h"

int op_sign(std::string_view c)
{
	assert(c == "L" || c == "R");
	return c == "L" ? -1 : 1;
}

void mod100(int& x) {
	x = (x % 100 + 100) % 100;
};
void nex_val_m1(int operand, const std::string_view& control_token, int& start, int& old, size_t& zero_count);
void next_val_m2(const std::string_view& control_token, int operand, int& start, size_t& zero_count, int& old);


int main()
{
	std::vector<std::string> in_data = read_input("C:\\source_code\\advent_of_code_2025\\day1\\input_data\\part_1.txt");
	//std::vector<std::string> in_data = read_input("C:\\source_code\\advent_of_code_2025\\day1\\input_data\\example.txt");
	int start = 50;
	size_t zero_count = 0;
	//Part1
	//...
	//Part2
	int old = 50;
	int start2 = 50;
	size_t zero_count2 = 0;
	int old2=0;
	for (std::string_view item : in_data)
	{
		std::string_view control_token = item.substr(0, 1);
		std::string_view value = item.substr(1);
		int operand;
		std::from_chars(value.data(), value.data() + value.size(), operand);
		nex_val_m1(operand, control_token, start, old, zero_count);
		next_val_m2(control_token, operand, start2, zero_count2, old2);
		if (zero_count != zero_count2)
		{
			std::cout << "jetzt\n";
		}
		old = start;
		old2 = start2;
	}
	std::cout << "Lösung Teil 2: " << zero_count << "\n";
	std::cout << "Lösung Teil 2: " << zero_count2 << "\n";

	old = 50;
	start = 50;
	zero_count = 0;
	for (std::string_view item : in_data)
	{
		std::string_view control_token = item.substr(0, 1);
		std::string_view value = item.substr(1);
		int operand;
		std::from_chars(value.data(), value.data() + value.size(), operand);
		next_val_m2(control_token, operand, start, zero_count, old);
	}
	std::cout << "Lösung Teil 2b: " << zero_count << "\n";

	return 0;
}

void next_val_m2(const std::string_view& control_token, int operand, int& start, size_t& zero_count, int& old)
{
	int dx = op_sign(control_token);
	for (int i = 0; i < operand; ++i) {
		start += dx;
		mod100(start);
		if (start == 0)
		{
			zero_count++;
		}
	}
}

void nex_val_m1(int operand, const std::string_view& control_token, int& start, int& old, size_t& zero_count)
{
	int rounds = operand / 100;
	int rest = operand % 100;
	operand *= op_sign(control_token);
	start += rest* op_sign(control_token);
	if (old !=0 && (start >= 100 || start <= 0))
	{
		zero_count++;
	}
	zero_count += rounds;
	mod100(start);
}
