#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include "matrix.h"
#include "load_data.h"
#include <unordered_map>
#include <chrono>



std::optional<tools::CharMatrix> try_place_shape_in_region(tools::CharMatrix region, const tools::CharMatrix& shape, size_t top, size_t left,char ch)
{
	if (top + shape.num_rows() > region.num_rows() || left + shape.num_cols() > region.num_cols())
	{
		return std::nullopt; // Shape does not fit in the region at the specified position
	}
	for (size_t r = 0; r < shape.num_rows(); ++r)
	{
		auto region_row = region.get_row(top + r);
		auto shape_row = shape.get_row(r);
		for (size_t c = 0; c < shape.num_cols(); ++c)
		{
			if (shape_row[c] != '.' && region_row[left + c] != '.')
			{
				return std::nullopt; // Overlap detected
			}
			else if (shape_row[c] != '.')
			{
				region_row[left + c] = ch; // Place shape character
			}
		}
	}
	//std::cout << "new_region: \n";
	//tools::print_charmatrix(region);
	return region; // Shape can be placed without overlap
}

using cache_t = std::unordered_map<tools::CharMatrix, size_t >; 

bool try_to_fit_shapes_in_region(const std::vector<Shape>& shapes, tools::CharMatrix region_matrix, size_t shape_idx, cache_t& cache)
{
	auto iter = cache.find(region_matrix);
	if (iter != cache.end())
	{
		if (iter->second >= shape_idx)
			return false;
	}
	size_t remaining_filled=0;
	for(size_t i=shape_idx;i<shapes.size();++i)
	{
		remaining_filled += shapes[i].shape.count_elements();
	}
	size_t free_in_region = region_matrix.num_cols()*region_matrix.num_rows() - region_matrix.count_elements();
	if (free_in_region < remaining_filled)
	{
		if (iter != cache.end())
		{
			if (iter->second < shape_idx)
			{
				iter->second = shape_idx;
			}
		}
		else
		{
			cache[region_matrix] = shape_idx;
		}
		return false;
	}
	for (size_t xt = 0; xt < region_matrix.num_cols(); ++xt)
	{
		for (size_t yt = 0; yt < region_matrix.num_rows(); ++yt)
		{
			for (const tools::CharMatrix& current_dev_shape : shapes[shape_idx].derived_shapes)
			{
				auto attempt = try_place_shape_in_region(region_matrix, current_dev_shape, yt, xt,'#');
				if (attempt)
				{
					auto next_shape_idx = shape_idx + 1;
					if (next_shape_idx == shapes.size())
					{
						//std::cout << "All shapes placed successfully.\n";
						return true;
					}
					/*std::cout << "new_region: \n";
					tools::print_charmatrix(*attempt);*/
					auto suc = try_to_fit_shapes_in_region(shapes, *attempt, next_shape_idx,cache);
					if (suc)
					{
						return true;
					}
				}
			}
		}
	}
	if (iter != cache.end())
	{
		if (iter->second < shape_idx)
		{
			iter->second = shape_idx;
		}
	}
	else
	{
		cache[region_matrix] = shape_idx;
	}
	return false;
}




int main()
{
	size_t can_fit_count = 0;
	std::vector<Shape> shapes;
	std::vector<Region> regions;
	try {
		load_input_file("C:/source_code/advent_of_code_2025/day12/input/input.txt", shapes, regions);
	} catch (const std::exception& ex) {
		std::cerr << "Error loading input file: " << ex.what() << "\n";
		return 1;
	}
	for (auto& shape : shapes)
	{
		shape.build_derived_shapes();
	}
	for (size_t i=0;i<regions.size();++i)
	{
		auto loop_start = std::chrono::steady_clock::now();
		const auto& region = regions[i];
		tools::CharMatrix region_matrix(region.height, region.width, '.');
		std::vector<Shape> shapes_to_fit;
		size_t id = 0;
		for (int shape_count : region.shapes_to_integrate)
		{
			auto it = std::find_if(shapes.begin(), shapes.end(), [&id](const Shape& s) { return s.id == id; });
			if (it != shapes.end())
			{
				for (int j = 0; j < shape_count;j++)
				{
					shapes_to_fit.push_back(*it);
				}
				/*if (shape_count > 0)
				{
					tools::print_charmatrix(it->shape);
				}*/
			}
			else
			{
				std::cerr << "Shape ID " << id << " not found.\n";
				exit(1);
			}
			++id;
		}
		
		cache_t cache;
		cache.rehash(std::numeric_limits<int16_t>::max());
		auto can_fit = try_to_fit_shapes_in_region(shapes_to_fit, region_matrix, 0,cache);
		if (can_fit)
		{
			++can_fit_count;
			std::cout << "Shapes can fit in the region of size " << region.width << "x" << region.height << "\n";
			//tools::print_charmatrix(*can_fit);
		}
		else
		{
			std::cout << "Shapes cannot fit in the region of size " << region.width << "x" << region.height << "\n";
		}
		auto loop_end = std::chrono::steady_clock::now();
		const auto loop_duration = std::chrono::duration_cast<std::chrono::milliseconds>(loop_end - loop_start).count();
		std::cout << "Region " << i << " processing time: " << loop_duration << " ms\n";
	}
	std::cout << "Total number of regions where shapes can fit: " << can_fit_count << "\n";
	return 0;
}















