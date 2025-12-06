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
	// Replace O(n^2) iterative merge with O(n log n): sort by start, then linear merge
    std::ranges::sort(ranges, [](const auto& a, const auto& b){ return a.get_first() < b.get_first(); });
    std::vector<tools::Range<tools::LongInt256>> merged;
    if (!ranges.empty())
    {
        tools::Range<tools::LongInt256> current = ranges.front();
        for (size_t i = 1; i < ranges.size(); ++i)
        {
            auto union_opt = current.union_with(ranges[i]);
            if (union_opt)
            {
                current = *union_opt; // merge into current
            }
            else
            {
                merged.push_back(current);
                current = ranges[i];
            }
        }
        merged.push_back(current);
    }
    ranges = std::move(merged);
    
	std::cout << "Number of merged ranges: " << ranges.size() <<  "\n";
    tools::LongInt256 num_of_items = 0;
	for (const auto& range : ranges){
        num_of_items += range.length();
	}
    std::cout << "Total number of items in all ranges: " << num_of_items << "\n";

	return 0;
}