#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>
#include "range.h"
#include "long-int.h"

template<typename T>
std::pair<std::vector<tools::Range<T>>, std::vector<T>> parse_input(const std::filesystem::path& filepath)
{
    std::ifstream input_file(filepath);
    if(!input_file.good())
    {
        throw std::runtime_error("Could not open file: " + filepath.string());
    }
    // Read ranges as strings in format "start-end" until an empty line is encountered.
    std::vector<tools::Range<T>> ranges;
    std::vector<T> items;
    std::string line;
    bool reading_ranges = true;
    while(std::getline(input_file, line))
    {
        // Treat lines that are empty or contain only whitespace as the separator
        auto first_non_ws = line.find_first_not_of(" \t\r\n");
        if (reading_ranges)
        {
            if (first_non_ws == std::string::npos)
            {
                // empty line -> switch to reading items
                reading_ranges = false;
                continue;
            }

            try
            {
                tools::Range<T> range(line);
                ranges.push_back(range);
            }
            catch(const std::invalid_argument& e)
            {
                std::cerr << "Skipping invalid range: " << line << " (" << e.what() << ")\n";
            }
        }
        else
        {
            // reading items: expect one integer per line
            if (first_non_ws == std::string::npos)
            {
                // skip additional blank lines
                continue;
            }
            try
            {
                T val = tools::string_to_integer<T>(line);
                items.push_back(static_cast<T>(val));
            }
            catch(const std::exception& e)
            {
                std::cerr << "Skipping invalid item line: " << line << " (" << e.what() << ")\n";
            }
        }
    }

    return {ranges, items};
}

int main()
{
    std::vector < tools::Range<tools::LongInt256>> ranges{};
    std::vector<tools::LongInt256> items{};
    std::tie(ranges,items) = parse_input<tools::LongInt256>("C:/source_code/advent_of_code_2025/day5/input/input.txt");
    size_t count = 0;
    for(const auto& item : items)
    {
        for(const auto& range : ranges)
        {
            if(range.is_in_range(item))
            {
                count++;
                break;
            }
        }
	}
    std::cout << "Number of items in any range: " << count << "\n";
    //Part 2
	bool something_erased = true;
    size_t round = 0;
	while (something_erased)
	{
		std::cout << "Merging round " << ++round << "\n";
		something_erased = false;
		for (size_t i = 0; i < ranges.size(); ++i)
		{
			std::vector<size_t> overlapping_ranges;
			tools::Range<tools::LongInt256>& result_range = ranges[i];
			for (size_t j = i + 1; j < ranges.size(); ++j)
			{
				auto union_range_opt = result_range.union_with(ranges[j]);
				//std::cout << "Checking ranges [" << result_range.get_first() << ", " << result_range.get_last() << "] and ["
				//	<< ranges[j].get_first() << ", " << ranges[j].get_last() << "]\n";
				if (union_range_opt)
				{
					//std::cout << " Merging into [" << union_range_opt->get_first() << ", " << union_range_opt->get_last() << "]\n";
					result_range = *union_range_opt;
					overlapping_ranges.push_back(j);
				}
			}
			std::ranges::sort(overlapping_ranges, std::greater<>());
			for (size_t idx : overlapping_ranges)
			{
				something_erased = true;
				std::swap(ranges[idx], ranges.back());
				ranges.pop_back();
			}
		}
	}
    
	std::cout << "Number of merged ranges: " << ranges.size() <<  "\n";
    tools::LongInt256 num_of_items = 0;
	for (const auto& range : ranges){
		//std::cout << "Range from" << range.get_first() << " to " << range.get_last() << ", Range length: " << range.length() << "\n";
        num_of_items += range.length();
	}
    std::cout << "Total number of items in all ranges: " << num_of_items << "\n";

	return 0;
}