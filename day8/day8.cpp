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
#include <map>
#include <set>


using uid_t = uint64_t;

template<typename T> 
constexpr T sqr(const T& v) {
	return v * v;
}

struct Point3d
{
	uid_t id;
	std::array<float, 3> coords;
	float& x() { return coords[0]; }
	float& y() { return coords[1]; }
	float& z() { return coords[2]; }
	const float& x() const { return coords[0]; }
	const float& y() const { return coords[1]; }
	const float& z() const { return coords[2]; }
};

float point_distance(const Point3d& p1, const Point3d&p2)
{
	return std::sqrt(
		sqr(p1.x() - p2.x()) + sqr(p1.y() - p2.y()) + sqr(p1.z() - p2.z()));
}


//Erstamal brute force, wenns nicht reicht können wir eine Gitterbasierten oder einen KD-Tree basierten Ansatz versuchen
std::tuple<uid_t,float> finde_next_neighbor(const Point3d& point, const std::vector<Point3d>& points)
{
	uid_t nearest_id = 0;
	float nearest_distance = std::numeric_limits<float>::max();
	for (const auto& other_point : points)
	{
		if (other_point.id == point.id)
			continue;
		float distance = point_distance(point, other_point);
		if (distance < nearest_distance)
		{
			nearest_distance = distance;
			nearest_id = other_point.id;
		}
	}
	return { nearest_id, nearest_distance };
}

std::vector<Point3d> parse_input(const std::string& filename)
{
	std::vector<Point3d> points{};
	points.reserve(1000);
	std::ifstream in(filename);
	if (!in.good())
	{
		std::cerr << "Error opening file: " << filename << std::endl;
		exit(1);
	}
	std::string line;
	uint64_t uid = 0;
	while (std::getline(in, line))
	{
		std::ranges::replace(line, ',', ' ');
		Point3d point;
		std::stringstream ss{ line };
		ss >> point.x();
		ss >> point.y();
		ss >> point.z();
		point.id = ++uid;
		points.push_back(point);
	}
	return points;
}


struct PairWithDistance
{
	uid_t id1;
	uid_t id2;
	float distance;
};

std::vector<PairWithDistance> calc_all_pairs(const std::vector<Point3d>& points)
{
	std::vector<PairWithDistance> all_pairs;
	all_pairs.reserve( points.size() * (points.size()-1)/2);
	for (size_t i=0;i<points.size(); ++i)
	{
		for (size_t j=i+1;j<points.size(); ++j)
		{
			float distance = point_distance(points[i], points[j]);
			PairWithDistance pwd{ points[i].id, points[j].id, distance };
			all_pairs.push_back(pwd);
		}
	}
	return all_pairs;
}
using circuit_t = std::set<uid_t>;
using v_circuit_t = std::vector<circuit_t>;

PairWithDistance connect_all(std::vector<PairWithDistance>& all_pairs, v_circuit_t& circuits, std::optional<size_t> break_at = std::nullopt);

int main()
{

	auto points_with_id = parse_input("C:/source_code/advent_of_code_2025/day8/input/input.txt");
	v_circuit_t circuits;
	for (const auto& point : points_with_id)
	{
		circuits.push_back(circuit_t{ point.id });
	}

	auto all_pairs = calc_all_pairs(points_with_id);
	std::ranges::sort(all_pairs, {}, &PairWithDistance::distance);

	auto lasst_pair = connect_all(all_pairs, circuits/*,1000*/); //uncomment break_at for part 1


	
	size_t check_product = 1;
	std::vector<size_t> circuit_sizes{};
	

	for (auto& cur_curcuit : circuits)
	{
		circuit_sizes.push_back(cur_curcuit.size());
	}
	std::ranges::sort(circuit_sizes, std::greater{});
	size_t index = 0;
	size_t break_at = 3;
	for (const auto& size : circuit_sizes)
	{
		//std::cout << "Circuit size: " << size << std::endl;
		check_product *= size;
		index++;
		if (index >= break_at)
			break;
	}

	std::cout << "Number of circuits: " << circuits.size() << std::endl;
	std::cout << "Product of sizes of the three largest circuits: " << check_product << std::endl;

	auto p1 = std::ranges::find(points_with_id, lasst_pair.id1, &Point3d::id);
	auto p2 = std::ranges::find(points_with_id, lasst_pair.id2, &Point3d::id);
	auto mul = static_cast<uint64_t>(p1->x()) * static_cast<uint64_t>(p2->x());
	std::cout << "Product of x-coordinates of last connected points: " << mul << std::endl;

	return 0;
}

PairWithDistance connect_all(std::vector<PairWithDistance>& all_pairs, v_circuit_t& circuits,std::optional<size_t> break_at)
{
	size_t index = 0;
	for (const auto& current_pair : all_pairs)
	{
		std::vector<size_t> index_of_containing_circuits{};
		for (auto& cur_curcuit : circuits)
		{
			if (cur_curcuit.contains(current_pair.id1) || cur_curcuit.contains(current_pair.id2))
			{
				cur_curcuit.insert(current_pair.id1);
				cur_curcuit.insert(current_pair.id2);
				index_of_containing_circuits.push_back(&cur_curcuit - &circuits[0]);
			}
		}
		//Falls keiner der Kreise die Punkte enthält, neuen Kreis anlegen
		if (index_of_containing_circuits.empty())
		{
			circuit_t circuit;
			circuit.insert(current_pair.id1);
			circuit.insert(current_pair.id2);
			circuits.push_back(circuit);
		}
		else if (index_of_containing_circuits.size() > 1)
		{
			//Kreise zusammenführen
			auto first_index = index_of_containing_circuits[0];
			for (size_t i = 1; i < index_of_containing_circuits.size(); ++i)
			{
				auto index_to_merge = index_of_containing_circuits[i];
				circuits[first_index].insert(circuits[index_to_merge].begin(), circuits[index_to_merge].end());
				circuits.erase(circuits.begin() + index_to_merge);
			}
		}
		index++;
		if (circuits.size() == 1 || ( break_at && *break_at <= index ))
		{
			return current_pair;
		}
	}
	std::unreachable();
}
