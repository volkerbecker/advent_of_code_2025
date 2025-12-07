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
#include "day7.h"
#include <map>


struct BeamPos
{
	size_t x;
	size_t y;
};
void sol_part_1(tools::CharMatrix chr_matrix);
size_t do_step(tools::CharMatrix chr_matrix, BeamPos pos);



int main()
{
	//tools::CharMatrix chr_matrix = tools::read_matrix_from_file("C:/source_code/advent_of_code_2025/day7/input/example.txt");
	tools::CharMatrix chr_matrix = tools::read_matrix_from_file("C:/source_code/advent_of_code_2025/day7/input/input.txt");
	std::cout << "Read matrix with " << chr_matrix.num_rows() << " rows and " << chr_matrix.num_cols() << " columns.\n";
	sol_part_1(chr_matrix);

	auto start = std::ranges::find(chr_matrix[0], 'S');
	size_t start_x = std::distance(chr_matrix[0].begin(), start);
	BeamPos start_pos{ start_x,0 };
	size_t total_splits = do_step(chr_matrix, start_pos);
	std::cout << "Num of lines: " << total_splits << "\n";
}

auto beampos_cmp = [](const BeamPos& a, const BeamPos& b) {
    return (a.y < b.y) || (a.y == b.y && a.x < b.x);
};
std::map<BeamPos, size_t, decltype(beampos_cmp)> cache{ beampos_cmp };

size_t do_step(tools::CharMatrix chr_matrix, BeamPos pos)
{
	auto iter = cache.find(pos);
	if (iter != cache.end())
	{
		return iter->second;
	}
	size_t new_y = pos.y + 1;
	size_t ret_val = 0;
	if (new_y == chr_matrix.num_rows() - 1)
	{
		ret_val = 1;
	}
	else if (chr_matrix[new_y][pos.x] == '.')
	{
		ret_val = do_step(chr_matrix, BeamPos{ pos.x,new_y });
	}
	else if (chr_matrix[new_y][pos.x] == '^')
	{
		size_t left_ways = do_step(chr_matrix, BeamPos{ pos.x - 1,new_y });
		size_t right_ways = do_step(chr_matrix, BeamPos{ pos.x + 1,new_y });
		ret_val = left_ways + right_ways;
	}
	else
	{
		std::cerr << "Hit invalid char at " << pos.x << "," << new_y << "\n";
		exit(1);
	}
	cache[pos] = ret_val;
	return ret_val;
}

void sol_part_1(tools::CharMatrix chr_matrix)
{
	size_t splits = 0;
	for (size_t line = 0; line<chr_matrix.num_rows() - 1; line++)
	{
		auto current_row = chr_matrix.get_row(line);
		auto next_row = chr_matrix.get_row(line + 1);
		for (size_t i = 0; i<current_row.size(); ++i)
		{
			if (current_row[i] == 'S')
			{
				current_row[i] = '|';
			}
			if (current_row[i] == '|')
			{
				if (next_row[i] == '.')
				{
					next_row[i] = '|';
				}
				else if (next_row[i] == '^')
				{
					next_row[i + 1] = '|';
					next_row[i - 1] = '|';
					splits++;
				}
			}
		}
		//tools::print_charmatrix(chr_matrix);
	}
	tools::print_charmatrix(chr_matrix);
	std::cout << "Total splits: " << splits << "\n";


}
