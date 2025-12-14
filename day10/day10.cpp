#include "loader.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include <limits>
#include <algorithm>
#include <ranges>
#include <helpers.h>

using jolt_point_t = std::vector<size_t>;




template<bool return_all=false>
auto find_shortest_ways_part_1(Configuration c)
{
    size_t min_presses = std::numeric_limits<size_t>::max();
    size_t best_combination = 0;
	std::vector<std::pair<size_t,size_t>> successful_ways_and_combinations;
    const size_t total_combinations = tools::pow2<size_t>(static_cast<unsigned int>(c.buttons.size()));
    for (size_t i = 0; i < total_combinations; ++i)
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
            if constexpr (return_all)
            {
                successful_ways_and_combinations.push_back({ presses, i });
            }
            if (presses < min_presses)
            {
                best_combination = i;
                min_presses = presses;
            }
        }
    }
    if constexpr (!return_all)
        return std::pair{ min_presses, best_combination };
    else
        return successful_ways_and_combinations;
}



std::vector<std::pair<size_t, size_t>> find_all_ways_to_even_jolts(Configuration c)
 {
     if (c.jolts.size() > 16)
     {
		 std::cerr << "Jolt size exceeds 16, not supported\n";
         exit(1);
     }
     c.target_state = 0;
     for(size_t i=0;i<c.jolts.size();i++)
     {
       
         if (c.jolts[c.jolts.size()-1-i] % 2 != 0)
         {
             //schiebe ein eins rein
             c.target_state = c.target_state << 1 | 1;
         }
		 else
		 {
			 c.target_state = c.target_state << 1;
		 }
     }
     //std::cout << std::bitset<16>(c.target_state) << '\n';
     return find_shortest_ways_part_1<true>(c);
 }

size_t find_shortest_way_to_zero_jolts(const Configuration& c)
 {
	auto ways = find_all_ways_to_even_jolts(c);
	size_t ret_steps_to_even = std::numeric_limits<size_t>::max();
    for (auto& [steps_to_even,way_to_even] : ways)
    {
        if (ret_steps_to_even == 0)
        {
            std::cout << "jetzt \n";
        }
        bool skip = false;
		//apply this way
        Configuration c_neu = c;
        //go the way to even jolts
        size_t current_combination = way_to_even;
        for (size_t j = 0; j < c_neu.buttons.size(); ++j)
        {
            if ((current_combination & 1) != 0)
            {
                bool ok = c_neu.press_button_to_joltage(j);
                if(!ok)
                { 
                    skip = true;
                    break;
                }
            }
            current_combination >>= 1;
        }
        if (skip)
			continue;
        //if alls are zero , we are done
        bool all_zero = std::ranges::all_of(c_neu.jolts, [](size_t x) { return x == 0; });
        if (all_zero)
        {
			ret_steps_to_even = std::min(ret_steps_to_even, steps_to_even);
            continue;
        }
        //now all jolts are even, divide by 2
        for (size_t i = 0; i < c_neu.jolts.size(); ++i)
        {
            c_neu.jolts[i] /= 2;
        }
        if (ret_steps_to_even == 0)
        {
            std::cout << "jetzt \n";
        }
        size_t steps_from_this = find_shortest_way_to_zero_jolts(c_neu);
        if(steps_from_this == std::numeric_limits<size_t>::max())
			continue;
		size_t steps_return_candiate = steps_to_even + 2*steps_from_this;
        ret_steps_to_even = std::min(ret_steps_to_even,steps_return_candiate);
        if (ret_steps_to_even == 0)
        {
			std::cout << "jetzt \n";
        }
    }
	return ret_steps_to_even;
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
 	    auto[result,_] = find_shortest_ways_part_1(cfg);
 		sum += result;
 	}
 	std::cout << "Sum of shortest ways: " << sum << "\n";
 	//part2
	// Die Lösungsidee stammt von hier:
	// https://www.reddit.com/r/adventofcode/comments/1pk87hl/2025_day_10_part_2_bifurcate_your_way_to_victory/
	sum = 0;
    for (const auto& cfg : config)
    {
        Configuration cfg_copy = cfg;
        auto val = find_shortest_way_to_zero_jolts(cfg_copy);
        if(val==0)
            std::cout << "jetzt \n";
		std::cout << "Shortest way to zero jolts: " << val << "\n";
		sum += val;
    }
	std::cout << "Sum of shortest ways to zero jolts: " << sum << "\n";
 }

