#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>

// node-id -> list of connected node-ids
std::map<std::string,std::vector<std::string>> network;
const std::string exit_point = "out";
const std::string entry_point = "you";

std::string trim(const std::string& value)
{
    auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) { return std::isspace(ch); });
    auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) { return std::isspace(ch); }).base();
    if (first >= last)
        return {};
    return { first, last };
}

void load_network(const std::vector<std::string>& lines)
{
    network.clear();
    for (const auto& raw_line : lines)
    {
        if (raw_line.empty())
            continue;

        auto colon_pos = raw_line.find(':');
        if (colon_pos == std::string::npos)
            continue; // Ignore malformed lines without delimiter

        std::string key = trim(raw_line.substr(0, colon_pos));
        if (key.empty())
            continue;

        std::string rest = trim(raw_line.substr(colon_pos + 1));
        std::vector<std::string> connections;
        if (!rest.empty())
        {
            std::istringstream iss(rest);
            std::string node;
            while (iss >> node)
                connections.push_back(node);
        }

        network[key] = std::move(connections);
    }
}

bool load_network_from_file(const std::string& file_path)
{
    std::ifstream input(file_path);
    if (!input)
        return false;

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line))
        lines.push_back(line);

    load_network(lines);
    return true;
}

bool do_log = false;
size_t ways_containing_boot = 0;
std::map<std::string, size_t> cache{};

size_t go_ways_impl(const std::string& start_node, const std::string& dst = exit_point, std::vector<std::string> path = {});


size_t go_ways(const std::string& start_node, const std::string& dst = exit_point, std::vector<std::string> path = {})
{
    cache.clear();
    ways_containing_boot = 0;
	return go_ways_impl(start_node, dst, path);
}

size_t go_ways_impl(const std::string& start_node, const std::string& dst, std::vector<std::string> path)
{
    if(auto iter=cache.find(start_node);iter!=cache.end())
    {
		return iter->second;
    }
	if (std::ranges::find(path,start_node) != path.end() )
	{
		std::cout << "Loop occours, code must adopted: " << start_node << "\n";
		exit(1);
	}
    path.push_back(start_node);
	if (start_node == dst)
	{
		if (do_log)
			std::cout << "Reached exit point: " << exit_point << "\n";
        if (std::ranges::find(path,"fft") != path.end() && std::ranges::find(path, "dac") != path.end())
        {
            ways_containing_boot++;
            if (do_log)
				std::cout << "Path contains boot nodes fft and dac\n";
        }
        cache[start_node]=1;
		return 1;
	}
	auto outs = network.find(start_node);
	if (outs == network.end())
	{
        if (do_log)
            std::cout << "We left the network \n";
        return 0;
	}
	size_t sum = 0;
	for (const auto& dest_node : outs->second)
	{
		if (do_log)
			std::cout << "From " << start_node << " to " << dest_node << "\n";
		sum += go_ways_impl(dest_node,dst,path);
	}
	cache[start_node] = sum;
    return sum;
}

int main()
{

   
   if (!load_network_from_file("C:/source_code/advent_of_code_2025/day11/input/input.txt"))
    {
       std::cerr << "Failed to read network file \n";
       return 1;
    }

    std::cout << "Loaded " << network.size() << " nodes\n";
    size_t number_of_ways = go_ways("svr");
    std::cout << "Number of ways to exit: " << number_of_ways << "\n";
	std::cout << "Number of ways exit containing boot nodes wihoutr cache hits  " << ways_containing_boot << "\n";
    
	size_t ways_from_fft_to_dac = go_ways("fft", "dac");
	std::cout << "Number of ways from fft to dac: " << ways_from_fft_to_dac << "\n";
	size_t ways_from_dac_to_fft = go_ways("dac", "fft");
	std::cout << "Number of ways from dac to fft: " << ways_from_dac_to_fft << "\n";
    if (ways_from_dac_to_fft > 0 && ways_from_fft_to_dac > 0)
    {
		std::cout << "Something strange here, both directions have ways, graph must contain loops\n";
		exit(1);
    }
	size_t total_ways_via_both = 0;
    if (ways_from_dac_to_fft > 0)
    {
		size_t ways_to_dac_node = go_ways("svr", "dac");
		std::cout << "Ways to dac node: " << ways_to_dac_node << "\n";
		size_t ways_from_fft_to_exit = go_ways("fft", "out");
		std::cout << "Ways from fft to exit: " << ways_from_fft_to_exit << "\n";
        total_ways_via_both = ways_to_dac_node * ways_from_dac_to_fft * ways_from_fft_to_exit;
    }
    else if (ways_from_fft_to_dac > 0)
	{
		size_t ways_to_fft_node = go_ways("svr", "fft");
		std::cout << "Ways to fft node: " << ways_to_fft_node << "\n";
        size_t ways_from_dac_to_exit = go_ways("dac", "out");
		std::cout << "Ways from dac to exit: " << ways_from_dac_to_exit << "\n";
		total_ways_via_both = ways_to_fft_node * ways_from_fft_to_dac * ways_from_dac_to_exit;  
    }
	std::cout << "Total number of ways via both boot nodes: " << total_ways_via_both << "\n";
	return 0;




}

