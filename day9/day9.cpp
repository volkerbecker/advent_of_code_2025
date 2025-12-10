#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>
#include "range.h"
#include "matrix.h"
#include <algorithm>
#include <ranges>
#include <limits>
#include <map>
#include <set>
#include <cmath>
#include <sstream>


using uid_t = uint64_t;

template<typename T> 
constexpr T sqr(const T& v) {
	return v * v;
}

struct Point64_t2d
{
	uid_t id;
	std::array<int64_t, 2> coords;
	int64_t& x() { return coords[0]; }
	int64_t& y() { return coords[1]; }
	const int64_t& x() const { return coords[0]; }
	const int64_t& y() const { return coords[1]; }
};

int64_t point64_t_pair_area(const Point64_t2d& p1, const Point64_t2d& p2)
{
    const auto width  = std::abs(p1.x() - p2.x()) + 1;
    const auto height = std::abs(p1.y() - p2.y()) + 1;
    return width * height;
}


struct PairWithArea
{
	uid_t id1;
	uid_t id2;
	int64_t area;
};

std::vector<PairWithArea> calc_all_pairs(const std::vector<Point64_t2d>& point64_ts)
{
	std::vector<PairWithArea> all_pairs;
	all_pairs.reserve( point64_ts.size() * (point64_ts.size()-1)/2);
	for (size_t i=0;i<point64_ts.size(); ++i)
	{
		for (size_t j=i+1;j<point64_ts.size(); ++j)
		{
			int64_t area = point64_t_pair_area(point64_ts[i], point64_ts[j]);
			PairWithArea pwd{ point64_ts[i].id, point64_ts[j].id, area };
			all_pairs.push_back(pwd);
		}
	}
	return all_pairs;
}

std::vector<Point64_t2d> load_point64_ts(const std::string& filename)
{
	std::vector<Point64_t2d> point64_ts;
	std::ifstream input(filename);
	if (!input.good()) {
		std::cerr << "Error opening file: " << filename << std::endl;
		exit(1);
	}
	uid_t id = 0;
	std::string line;
	while (std::getline(input, line))
	{
		if (line.empty()) continue;
		// Kommata als Trennzeichen ersetzen
		std::ranges::replace(line, ',', ' ');
		std::stringstream ss(line);
		Point64_t2d p;
		if (!(ss >> p.x() >> p.y())) continue; // überspringe ungültige Zeilen
		p.id = id++;
		point64_ts.push_back(p);
	}
	return point64_ts;
}

void fill_area_in_cahrmatrix(tools::CharMatrix& matrix, const Point64_t2d& startpoint)
{
	std::vector<std::pair<int64_t, int64_t>> queue;
	std::set<std::pair<int64_t, int64_t>> visited;
	
	queue.push_back({startpoint.y(), startpoint.x()});
	visited.insert({startpoint.y(), startpoint.x()});
	
	while (!queue.empty())
	{
		auto [y, x] = queue.back();
		queue.pop_back();
		
		// Prüfe ob Position innerhalb der Matrix liegt
		if (y < 0 || y >= static_cast<int64_t>(matrix.num_rows()) || 
		    x < 0 || x >= static_cast<int64_t>(matrix.num_cols()))
			continue;
		
		// Prüfe ob Position bereits eine Wand ist
		if (matrix[y][x] == '#')
			continue;
		
		// Fülle die Position
		matrix[y][x] = '#';
		
		// Füge orthogonale Nachbarn zur Queue hinzu
		std::array<std::pair<int64_t, int64_t>, 4> neighbors = {{
			{y - 1, x},     // oben
			{y + 1, x},     // unten
			{y, x - 1},     // links
			{y, x + 1}      // rechts
		}};
		
		for (const auto& [ny, nx] : neighbors)
		{
			if (visited.find({ny, nx}) == visited.end())
			{
				visited.insert({ny, nx});
				queue.push_back({ny, nx});
			}
		}
	}
}

bool is_rectangle_filled(const tools::CharMatrix& matrix, const Point64_t2d& p1, const Point64_t2d& p2)
{
	int64_t min_x = std::min(p1.x(), p2.x());
	int64_t max_x = std::max(p1.x(), p2.x());
	int64_t min_y = std::min(p1.y(), p2.y());
	int64_t max_y = std::max(p1.y(), p2.y());
	
	for (int64_t y = min_y; y <= max_y; ++y)
	{
		if (y < 0 || y >= static_cast<int64_t>(matrix.num_rows()))
			return false;
		
		for (int64_t x = min_x; x <= max_x; ++x)
		{
			if (x < 0 || x >= static_cast<int64_t>(matrix.num_cols()))
				return false;
			
			if (matrix[y][x] != '#')
				return false;
		}
	}
	
	return true;
}

int main()
{
	auto point64_ts = load_point64_ts("C:/source_code/advent_of_code_2025/day9/input/input.txt");
	auto all_pairs = calc_all_pairs(point64_ts);
	std::ranges::sort(all_pairs, std::greater<>{}, &PairWithArea::area);
	//auto max_pair = std::ranges::max(all_pairs, {}, &PairWithArea::area);
	auto max_pair = all_pairs.front();
	std::cout << "Max area: " << max_pair.area << std::endl;
	std::cout << "Between points ID " << max_pair.id1 << " and ID " << max_pair.id2 << std::endl;
	auto p1 = std::ranges::find(point64_ts, max_pair.id1, &Point64_t2d::id);
	auto p2 = std::ranges::find(point64_ts, max_pair.id2, &Point64_t2d::id);
	std::cout << "Point 1 coordinates: (" << p1->x() << ", " << p1->y() << ")" << std::endl;
	std::cout << "Point 2 coordinates: (" << p2->x() << ", " << p2->y() << ")" << std::endl;


	//Part2
	int64_t max_x = std::ranges::max(point64_ts, {}, [](auto& pnt) {return pnt.x(); }).x();
	int64_t min_x = std::ranges::min(point64_ts, {}, [](auto& pnt) {return pnt.x(); }).x();
	int64_t max_y = std::ranges::max(point64_ts, {}, [](auto& pnt) {return pnt.y(); }).y();
	int64_t min_y = std::ranges::min(point64_ts, {}, [](auto& pnt) {return pnt.y(); }).y();
	std::ranges::for_each(point64_ts, [&](auto& pnt) {
		pnt.x() = pnt.x() - min_x;
		pnt.y() = pnt.y() - min_y;
		});
	max_x -= min_x;
	max_y -= min_y;
	tools::CharMatrix floor(max_y + 1, max_x + 1, '.');
	for (size_t i=1;i<=point64_ts.size();++i)
	{
		const auto& point1 = point64_ts[i - 1];
		const auto& point2 = i < point64_ts.size() ? point64_ts[i] : point64_ts[0];
		if (point1.x() == point2.x())
		{
			for(int64_t y = std::min(point1.y(), point2.y()); y <= std::max(point1.y(), point2.y()); ++y)
			{
				floor[y][point1.x()] = '#';
			}
		} else if (point1.y() == point2.y())
		{
			for (int64_t x = std::min(point1.x(), point2.x()); x <= std::max(point1.x(), point2.x()); ++x)
			{
				floor[point1.y()][x] = '#';
			}
		} 
		else
		{
			std::cerr << "Non-straight line between points ID " << point1.id << " and ID " << point2.id << std::endl;
		}
	}
	//tools::print_charmatrix(floor);
	
	// Bestimme inneren Startpunkt für Flood-Fill
	if (point64_ts.size() >= 3)
	{
		const auto& p1 = point64_ts[0];
		const auto& p2 = point64_ts[1];
		const auto& p3 = point64_ts[2];
		
		// Richtung von p1 zu p2
		int64_t dir1_x = (p2.x() > p1.x()) ? 1 : (p2.x() < p1.x()) ? -1 : 0;
		int64_t dir1_y = (p2.y() > p1.y()) ? 1 : (p2.y() < p1.y()) ? -1 : 0;
		
		// Richtung von p2 zu p3
		int64_t dir2_x = (p3.x() > p2.x()) ? 1 : (p3.x() < p2.x()) ? -1 : 0;
		int64_t dir2_y = (p3.y() > p2.y()) ? 1 : (p3.y() < p2.y()) ? -1 : 0;
		
		// Startpunkt: p1 + dir1 + dir2
		Point64_t2d inner_start;
		inner_start.x() = p1.x() + dir1_x + dir2_x;
		inner_start.y() = p1.y() + dir1_y + dir2_y;
		inner_start.id = 0;
		
		std::cout << "Inner start point: (" << inner_start.x() << ", " << inner_start.y() << ")" << std::endl;
		
		// Fülle den inneren Bereich
		fill_area_in_cahrmatrix(floor, inner_start);
		
		/*std::cout << "\nAfter filling:\n";
		tools::print_charmatrix(floor);*/
		
		// Finde das größte Rechteck, das vollständig mit # gefüllt ist
		int64_t max_filled_area = 0;
		PairWithArea max_filled_pair{0, 0, 0};
		
		for (const auto& pair : all_pairs)
		{
			auto it1 = std::ranges::find(point64_ts, pair.id1, &Point64_t2d::id);
			auto it2 = std::ranges::find(point64_ts, pair.id2, &Point64_t2d::id);
			
			if (it1 != point64_ts.end() && it2 != point64_ts.end())
			{
				if (is_rectangle_filled(floor, *it1, *it2))
				{
					max_filled_area = pair.area;
					max_filled_pair = pair;
					break;
				}
			}
		}
		
		std::cout << "\nMax filled area: " << max_filled_area << std::endl;
		std::cout << "Between points ID " << max_filled_pair.id1 << " and ID " << max_filled_pair.id2 << std::endl;
		
		if (max_filled_area > 0)
		{
			auto it1 = std::ranges::find(point64_ts, max_filled_pair.id1, &Point64_t2d::id);
			auto it2 = std::ranges::find(point64_ts, max_filled_pair.id2, &Point64_t2d::id);
			std::cout << "Point 1 coordinates: (" << it1->x() << ", " << it1->y() << ")" << std::endl;
			std::cout << "Point 2 coordinates: (" << it2->x() << ", " << it2->y() << ")" << std::endl;
		}
	}

	return 0;
}

