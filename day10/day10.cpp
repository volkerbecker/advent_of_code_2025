#include <queue>
#include "loader.h"
#include <set>
#include <iostream>
#include <map>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <unordered_map>
#include <limits>
#include <algorithm>
#include <cstddef>

#include <vector>
#include <queue>
#include <limits>
#include <cstdint>
#include <algorithm>
#include <numeric>

#include <vector>
#include <limits>
#include <cstdint>
#include <algorithm>
#include <helpers.h>

using jolt_point_t = std::vector<size_t>;




size_t find_shortest_ways_part_1(Configuration c) 
{
    size_t min_presses = std::numeric_limits<size_t>::max();
    for (size_t i = 0; i < std::pow(2,c.buttons.size()); ++i)
    {
        size_t current_combination = i;
        c.curr_state = 0;
		size_t presses = 0;
		
        for (size_t j = 0; j < c.buttons.size(); ++j)
        {
            if ((current_combination & 1) != 0)
            {
				presses++;
                c.press_button(j);
            }
			current_combination >>= 1;
        }
        if (c.curr_state == c.target_state)
        {
            if (presses < min_presses)
            {
				min_presses = presses;
            }
            return std::popcount(current_combination);
		}
    }
	return min_presses;
}
 


 int main()
 {
 	//part1
 	size_t sum = 0;
 	//std::vector<Configuration> config = load_configurations("C:/source_code/advent_of_code_2025/day10/input/example.txt");
    std::vector<Configuration> config = load_configurations("C:/source_code/advent_of_code_2025/day10/input/input.txt");
 	for (const auto& cfg : config)
 	{
 		//std::cout << "Config: target_state=" << cfg.target_state << ", buttons=" << cfg.buttons.size() << ", jolts=" << cfg.jolts.size() << "\n";
 		size_t  result = find_shortest_ways_part_1(cfg);
 		sum += result;
 	}
 	std::cout << "Sum of shortest ways: " << sum << "\n";
 	//part2 
 }

