#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>
#include "range.h"
#include "long-int.h"
#include "matrix.h"
#include <algorithm>
#include <ranges>
#include <limits>


std::tuple<tools::Matrix<int64_t>, std::vector<char>> parse_input(const std::filesystem::path& filepath)
{
	std::ifstream input_file(filepath);
	if (input_file.good() == false)
	{
		throw std::runtime_error("Could not open file: " + filepath.string());
	}
	// Read matrix of integers until a line is empty or contains only '+' or '*'.

	std::vector<int64_t> matrix_data;
	size_t num_rows = 0;
	size_t num_cols = 0;
	std::string line;
	std::vector<char> operators;
	while (std::getline(input_file, line))
	{
		 // trim whitespace
		auto first = line.find_first_not_of(" \t\r\n");
		if (first == std::string::npos)
		{
			break; // empty line -> stop
		}
		auto last = line.find_last_not_of(" \t\r\n");
		std::string trimmed = line.substr(first, last - first + 1);

		bool operator_line = (trimmed.front() == '+' || trimmed.front() == '*');

		 // parse space-separated integers
		std::istringstream iss(trimmed);

		size_t col = 0;
		if (operator_line)
		{
			char val;
			while (iss >> val)
			{
				operators.push_back(val);
				++col;
			}
		}
		else 
		{
			int64_t val;
			while (iss >> val)
			{
				matrix_data.push_back(val);
				++col;
			}
		}

		if (num_cols == 0)
		{
			num_cols = col;
		}
		else if (col != num_cols)
		{
			throw std::runtime_error("Inconsistent number of columns in matrix input");
		}
		if(!operator_line)
			++num_rows;
	}

	tools::Matrix<int64_t> mat(num_rows, num_cols, std::move(matrix_data));
	if(operators.size() != num_cols)
	{
		throw std::runtime_error("Number of operators does not match number of matrix columns");
	}
	return { mat, operators };
}


int64_t parse_input_part2(const std::filesystem::path& filepath)
{
	tools::CharMatrix matrix = tools::read_matrix_from_file(filepath.string());
	auto operators_row = matrix.pop_last_row();
	matrix.transpose();
	char current_operator{};
	int64_t result = 0;
	int64_t partial_result = 0;
	for (size_t i = 0; i < matrix.num_rows(); ++i)
	{
		if (operators_row[i] == '+' || operators_row[i] == '*')
		{
			current_operator = operators_row[i];	
			if ((partial_result > 0 && result > (std::numeric_limits<int64_t>::max() - partial_result)) ||
				(partial_result < 0 && result < (std::numeric_limits<int64_t>::min() - partial_result)))
			{
				std::cerr << "Overflow detected while adding result to result (result=" << result
					<< ", partial_result=" << partial_result  << ")\n";
				return 1;
			}
			result += partial_result;
			std::cout << "partial result: " << partial_result << ", total result: " << result << "\n";
			partial_result = current_operator == '+' ? 0 : 1;
		}
		std::stringstream ss;
		for (size_t j = 0; j < matrix[i].size();++j)
			ss << matrix[i][j];
		auto first = ss.str().find_first_not_of(" \t\r\n");
		if (first == std::string::npos)
		{
			continue;
		}
		auto last = ss.str().find_last_not_of(" \t\r\n");
		std::string trimmed = ss.str().substr(first, last - first + 1);
		if (trimmed.empty())
			continue;
		int64_t val = tools::string_to_integer<int64_t>(trimmed);
		if (current_operator == '+')
		{
			partial_result += val;
		}
		else if (current_operator == '*')
		{
			partial_result *= val;
		}
	}
	result += partial_result;
	std::cout << "partial result: " << partial_result << ", total result: " << result << "\n";

	return result;
}


int main()
{
	tools::Matrix<int64_t> mat{};
	std::vector<char> operators{};
	std::tie(mat, operators) = parse_input("C:/source_code/advent_of_code_2025/day6/input/input.txt");
	mat.transpose();
	int64_t chekl_sum = 0;
	for (size_t i = 0; i < operators.size(); ++i)
	{
		auto op = operators[i] == '+' ? [](int64_t a, int64_t b) { return a + b; } : [](int64_t a, int64_t b) { return a * b; };
		auto result = std::ranges::fold_left(mat[i], int64_t{ operators[i] == '+' ? 0 : 1 }, op);
		// Overflow-Check für Addition (chekl_sum + result)
		if ((result > 0 && chekl_sum > (std::numeric_limits<int64_t>::max() - result)) ||
			(result < 0 && chekl_sum < (std::numeric_limits<int64_t>::min() - result)))
		{
			std::cerr << "Overflow detected while adding result to checksum (chekl_sum=" << chekl_sum
					  << ", result=" << result << ")\n";
			return 1;
		}
		chekl_sum += result;
	}
	std::cout << "Checksum: " << chekl_sum << "\n";

	auto result = parse_input_part2("C:/source_code/advent_of_code_2025/day6/input/input.txt");
	std::cout << "Part 2 result: " << result << "\n";
}