#include <iostream>
#include <filesystem>
#include <fstream>
#include "matrix.h"


// laut aufgabe sind valide Rollen Einträge bei denen ein @ steht und weniger als 4 der acht nachbarn ein @ enthalten
bool is_valid_role(tools::CharMatrix& matrix, int row, int col,bool remove=false)
{
	if (matrix[row][col] != '@')
		return false;
	int neighbor_count = 0;
	for (int dr = -1; dr <= 1; ++dr)
	{
		for (int dc = -1; dc <= 1; ++dc)
		{
			int test_row = row + dr;
			int test_col = col + dc;
			if ((dr == 0 && dc == 0) || test_col < 0 || test_row < 0 || test_col >= matrix.num_cols() || test_row >= matrix.num_rows())
				continue; // skip self and out-of-bounds
			if (matrix[test_row][test_col] == '@')
			{
				neighbor_count++;
				if (neighbor_count >= 4)
					return false;
			}
		}
	}
	if(remove)
		matrix[row][col] = '.';
	return true;
}

int main()
{

	//Part 1
	tools::CharMatrix input = tools::read_matrix_from_file("C:/source_code/advent_of_code_2025/day4/input/input.txt");
	size_t num_of_valid_roles = 0;
	for (size_t r = 0; r < input.num_rows(); ++r) {
		for(size_t c=0;c<input.num_cols();++c){
			if(is_valid_role(input, r, c)){
				num_of_valid_roles++;
			}
		}
	}
	std::cout << "Number of removeable roles: " << num_of_valid_roles << "\n";

	size_t num_of_toltal_removed_rolls = 0;
	//Part 2, es sollen alle entfernbaren Rollen entfernt werden, bis keine mehr entfernt werden können
	do {
		num_of_valid_roles = 0;
		for (size_t r = 0; r < input.num_rows(); ++r) {
			for (size_t c = 0; c < input.num_cols(); ++c) {
				if (is_valid_role(input, r, c, true)) {
					num_of_valid_roles++;
				}
			}
		}
		num_of_toltal_removed_rolls += num_of_valid_roles;
	} while (num_of_valid_roles > 0);

	std::cout << "Total number of removed roles: " << num_of_toltal_removed_rolls << "\n";


	return 0;
}