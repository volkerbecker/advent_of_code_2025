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

uid_t generate_id() {
	static uid_t id = 0;
	return id++;
};

struct Point64_t2d
{
public:
	Point64_t2d() = default;
	Point64_t2d(int64_t x_, int64_t y_) {
		x() = x_;
		y() = y_;
	}

	std::array<int64_t, 2> coords;
	int64_t& x() { return coords[0]; }
	int64_t& y() { return coords[1]; }
	const int64_t& x() const { return coords[0]; }
	const int64_t& y() const { return coords[1]; }
	uid_t get_id() const noexcept { return id; }
private:
		uid_t id = generate_id();
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
			PairWithArea pwd{ point64_ts[i].get_id(), point64_ts[j].get_id(), area };
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
		point64_ts.push_back(p);
	}
	return point64_ts;
}

bool is_polygon_convex(const std::vector<Point64_t2d>& polygon)
{
	if (polygon.size() < 3)
		return false;
	
	size_t n = polygon.size();
	bool has_positive = false;
	bool has_negative = false;
	
	for (size_t i = 0; i < n; ++i)
	{
		const auto& p1 = polygon[i];
		const auto& p2 = polygon[(i + 1) % n];
		const auto& p3 = polygon[(i + 2) % n];
		
		// Berechne Kreuzprodukt der Vektoren (p2-p1) x (p3-p2)
		int64_t dx1 = p2.x() - p1.x();
		int64_t dy1 = p2.y() - p1.y();
		int64_t dx2 = p3.x() - p2.x();
		int64_t dy2 = p3.y() - p2.y();
		
		int64_t cross_product = dx1 * dy2 - dy1 * dx2;
		
		if (cross_product > 0)
			has_positive = true;
		else if (cross_product < 0)
			has_negative = true;
		
		// Wenn beide Vorzeichen vorkommen, ist das Polygon konkav
		if (has_positive && has_negative)
			return false;
	}
	
	return true;
}

void print_polygon_scaled(const std::vector<Point64_t2d>& polygon, size_t max_width = 100, const std::vector<Point64_t2d>* overlay = nullptr)
{
    const bool has_primary = !polygon.empty();
    const bool has_overlay = overlay != nullptr && !overlay->empty();
    if (!has_primary && !has_overlay) {
        std::cout << "(Empty polygon)\n";
        return;
    }

    int64_t min_x = 0;
    int64_t max_x = 0;
    int64_t min_y = 0;
    int64_t max_y = 0;
    bool bounds_initialized = false;
    auto include_point = [&](const Point64_t2d& p) {
        if (!bounds_initialized) {
            min_x = max_x = p.x();
            min_y = max_y = p.y();
            bounds_initialized = true;
            return;
        }
        min_x = std::min(min_x, p.x());
        max_x = std::max(max_x, p.x());
        min_y = std::min(min_y, p.y());
        max_y = std::max(max_y, p.y());
    };
    auto include_polygon = [&](const std::vector<Point64_t2d>& poly) {
        for (const auto& p : poly) {
            include_point(p);
        }
    };

    if (has_primary) {
        include_polygon(polygon);
    }
    if (has_overlay) {
        include_polygon(*overlay);
    }

    int64_t actual_width = max_x - min_x + 1;
    int64_t actual_height = max_y - min_y + 1;

    std::cout << "Polygon bounds: " << actual_width << "x" << actual_height
              << " (from [" << min_x << "," << min_y << "] to ["
              << max_x << "," << max_y << "])\n";

    double scale = 1.0;
    if (actual_width > static_cast<int64_t>(max_width)) {
        scale = static_cast<double>(actual_width) / max_width;
    }

    size_t output_width = static_cast<size_t>((actual_width + scale - 1) / scale);
    size_t output_height = static_cast<size_t>((actual_height + scale - 1) / scale);

    if (scale > 1.0) {
        std::cout << "Scaling by " << scale << " to fit in "
                  << output_width << "x" << output_height << "\n";
    }

    std::vector<std::vector<char>> grid(output_height, std::vector<char>(output_width, '.'));

    auto plot = [&](int64_t y, int64_t x, char mark) {
        if (y < 0 || y >= static_cast<int64_t>(output_height) ||
            x < 0 || x >= static_cast<int64_t>(output_width)) {
            return;
        }
        char& cell = grid[y][x];
        if (cell == '.' || cell == mark) {
            cell = mark;
        } else {
            cell = 'X'; // mark overlaps
        }
    };

    auto draw_polygon = [&](const std::vector<Point64_t2d>& poly, char mark) {
        if (poly.empty()) {
            return;
        }
        for (size_t i = 0; i < poly.size(); ++i) {
            const auto& p1 = poly[i];
            const auto& p2 = poly[(i + 1) % poly.size()];

            int64_t x1 = static_cast<int64_t>((p1.x() - min_x) / scale);
            int64_t y1 = static_cast<int64_t>((p1.y() - min_y) / scale);
            int64_t x2 = static_cast<int64_t>((p2.x() - min_x) / scale);
            int64_t y2 = static_cast<int64_t>((p2.y() - min_y) / scale);

            int64_t dx = std::abs(x2 - x1);
            int64_t dy = std::abs(y2 - y1);
            int64_t sx = (x1 < x2) ? 1 : -1;
            int64_t sy = (y1 < y2) ? 1 : -1;
            int64_t err = dx - dy;

            int64_t x = x1;
            int64_t y = y1;

            while (true) {
                plot(y, x, mark);

                if (x == x2 && y == y2) {
                    break;
                }

                int64_t e2 = 2 * err;
                if (e2 > -dy) {
                    err -= dy;
                    x += sx;
                }
                if (e2 < dx) {
                    err += dx;
                    y += sy;
                }
            }
        }
    };

    if (has_primary) {
        draw_polygon(polygon, '#');
    }
    if (has_overlay) {
        draw_polygon(*overlay, '*');
        std::cout << "Overlay polygon drawn using '*' (overlaps marked as 'X').\n";
    }

    for (size_t y = 0; y < output_height; ++y) {
        for (size_t x = 0; x < output_width; ++x) {
            std::cout << grid[y][x];
        }
        std::cout << '\n';
    }
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

std::vector<Point64_t2d> get_corners(Point64_t2d p1, Point64_t2d p2)
{
	std::vector<Point64_t2d> result = {
		Point64_t2d(p1.x(), p1.y()),
		Point64_t2d(p2.x(), p1.y()),
		Point64_t2d(p2.x(), p2.y()),
		Point64_t2d(p1.x(), p2.y())
	};
	return result;
}

bool line_intersects_polygon_perpendicularly(const std::vector<Point64_t2d>& polygon, const Point64_t2d& p1, const Point64_t2d& p2)
{
	// Prüfe ob Verbindungslinie horizontal oder vertikal ist
	bool is_horizontal = (p1.y() == p2.y());
	bool is_vertical = (p1.x() == p2.x());
	
	if (!is_horizontal && !is_vertical)
		return false; // Linie ist weder horizontal noch vertikal
	
	int64_t line_min_x = std::min(p1.x(), p2.x());
	int64_t line_max_x = std::max(p1.x(), p2.x());
	int64_t line_min_y = std::min(p1.y(), p2.y());
	int64_t line_max_y = std::max(p1.y(), p2.y());
	
	// Iteriere über alle Kanten des Polygons
	for (size_t i = 0; i < polygon.size(); ++i)
	{
		const auto& edge_p1 = polygon[i];
		const auto& edge_p2 = polygon[(i + 1) % polygon.size()];
		
		// Prüfe ob Polygonkante horizontal oder vertikal ist
		bool edge_is_horizontal = (edge_p1.y() == edge_p2.y());
		bool edge_is_vertical = (edge_p1.x() == edge_p2.x());
		
		if (!edge_is_horizontal && !edge_is_vertical)
			continue; // Überspringe nicht-orthogonale Kanten
		
		// Prüfe auf senkrechte Kreuzung
		if (is_horizontal && edge_is_vertical)
		{
			// Verbindungslinie ist horizontal, Polygonkante ist vertikal
			int64_t edge_x = edge_p1.x();
			int64_t edge_min_y = std::min(edge_p1.y(), edge_p2.y());
			int64_t edge_max_y = std::max(edge_p1.y(), edge_p2.y());
			int64_t line_y = p1.y();
			
			// Prüfe ob sich die Linien schneiden (ohne Endpunkte)
			if (edge_x > line_min_x && edge_x < line_max_x &&
			    line_y > edge_min_y && line_y < edge_max_y)
			{
				return true;
			}
		}
		else if (is_vertical && edge_is_horizontal)
		{
			// Verbindungslinie ist vertikal, Polygonkante ist horizontal
			int64_t edge_y = edge_p1.y();
			int64_t edge_min_x = std::min(edge_p1.x(), edge_p2.x());
			int64_t edge_max_x = std::max(edge_p1.x(), edge_p2.x());
			int64_t line_x = p1.x();
			
			// Prüfe ob sich die Linien schneiden (ohne Endpunkte)
			if (edge_y > line_min_y && edge_y < line_max_y &&
			    line_x > edge_min_x && line_x < edge_max_x)
			{
				return true;
			}
		}
	}
	
	return false;
}

bool point_on_segment(const Point64_t2d& a, const Point64_t2d& b, const Point64_t2d& p)
{
    if (a.x() == b.x()) {
        if (p.x() != a.x()) {
            return false;
        }
        const int64_t min_y = std::min(a.y(), b.y());
        const int64_t max_y = std::max(a.y(), b.y());
        return p.y() >= min_y && p.y() <= max_y;
    }

    if (a.y() == b.y()) {
        if (p.y() != a.y()) {
            return false;
        }
        const int64_t min_x = std::min(a.x(), b.x());
        const int64_t max_x = std::max(a.x(), b.x());
        return p.x() >= min_x && p.x() <= max_x;
    }

    const int64_t cross = (b.x() - a.x()) * (p.y() - a.y()) - (b.y() - a.y()) * (p.x() - a.x());
    if (cross != 0) {
        return false;
    }

    const int64_t min_x = std::min(a.x(), b.x());
    const int64_t max_x = std::max(a.x(), b.x());
    const int64_t min_y = std::min(a.y(), b.y());
    const int64_t max_y = std::max(a.y(), b.y());
    return (p.x() >= min_x && p.x() <= max_x && p.y() >= min_y && p.y() <= max_y);
}

bool point_in_or_on_polygon(const std::vector<Point64_t2d>& polygon, const Point64_t2d& point)
{
    const size_t n = polygon.size();
    if (n == 0) {
        return false;
    }
    if (n == 1) {
        return polygon.front().x() == point.x() && polygon.front().y() == point.y();
    }

    bool inside = false;
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        const auto& a = polygon[j];
        const auto& b = polygon[i];

        if (point_on_segment(a, b, point)) {
            return true;
        }

        const bool crosses_ray = ((a.y() > point.y()) != (b.y() > point.y()));
        if (crosses_ray && a.x() == b.x() && point.x() < a.x()) {
            inside = !inside;
        }
    }
    return inside;
}

int main()
{
	auto point64_ts = load_point64_ts("C:/source_code/advent_of_code_2025/day9/input/input.txt");
	bool is_convex = is_polygon_convex(point64_ts);
	std::cout << "Polygon is " << (is_convex ? "convex" : "not convex") << std::endl;

	std::cout << "\nDirect polygon visualization:\n";
	print_polygon_scaled(point64_ts, 200);

	auto all_pairs = calc_all_pairs(point64_ts);
	std::ranges::sort(all_pairs, std::greater<>{}, &PairWithArea::area);
	//auto max_pair = std::ranges::max(all_pairs, {}, &PairWithArea::area);
	auto max_pair = all_pairs.front();
	std::cout << "Max area: " << max_pair.area << std::endl;
	std::cout << "Between points ID " << max_pair.id1 << " and ID " << max_pair.id2 << std::endl;
	auto p1 = std::ranges::find(point64_ts, max_pair.id1, &Point64_t2d::get_id);
	auto p2 = std::ranges::find(point64_ts, max_pair.id2, &Point64_t2d::get_id);
	std::cout << "Point 1 coordinates: (" << p1->x() << ", " << p1->y() << ")" << std::endl;
	std::cout << "Point 2 coordinates: (" << p2->x() << ", " << p2->y() << ")" << std::endl;


	//Part2 
	for (const auto& current_pair : all_pairs)
	{
		Point64_t2d& p1 = *std::ranges::find(point64_ts,current_pair.id1,  &Point64_t2d::get_id);
		Point64_t2d& p2 = *std::ranges::find(point64_ts, current_pair.id2, &Point64_t2d::get_id);
		auto corners = get_corners(p1, p2);
		corners.push_back(corners[0]); // Schliesse den Kreis
		bool intersects = false;
		for (size_t i = 1; i < corners.size(); ++i)
		{
			auto c1 = corners[i - 1];
			auto c2 = corners[i];
			intersects = line_intersects_polygon_perpendicularly(point64_ts, c1, c2) || !point_in_or_on_polygon(point64_ts, c1) || !point_in_or_on_polygon(point64_ts, c2);
			if (intersects)
				break;
		}
		if (!intersects)
		{
			std::cout << "The biggest areas inside the polygon is: " << current_pair.area << "\n";
			print_polygon_scaled(point64_ts, 200,&corners);
			break;
		}
	}


	////Part2
	//int64_t max_x = std::ranges::max(point64_ts, {}, [](auto& pnt) {return pnt.x(); }).x();
	//int64_t min_x = std::ranges::min(point64_ts, {}, [](auto& pnt) {return pnt.x(); }).x();
	//int64_t max_y = std::ranges::max(point64_ts, {}, [](auto& pnt) {return pnt.y(); }).y();
	//int64_t min_y = std::ranges::min(point64_ts, {}, [](auto& pnt) {return pnt.y(); }).y();
	//std::ranges::for_each(point64_ts, [&](auto& pnt) {
	//	pnt.x() = pnt.x() - min_x;
	//	pnt.y() = pnt.y() - min_y;
	//	});
	//max_x -= min_x;
	//max_y -= min_y;
	//tools::CharMatrix floor(max_y + 1, max_x + 1, '.');
	//for (size_t i=1;i<=point64_ts.size();++i)
	//{
	//	const auto& point1 = point64_ts[i - 1];
	//	const auto& point2 = i < point64_ts.size() ? point64_ts[i] : point64_ts[0];
	//	if (point1.x() == point2.x())
	//	{
	//		for(int64_t y = std::min(point1.y(), point2.y()); y <= std::max(point1.y(), point2.y()); ++y)
	//		{
	//			floor[y][point1.x()] = '#';
	//		}
	//	} else if (point1.y() == point2.y())
	//	{
	//		for (int64_t x = std::min(point1.x(), point2.x()); x <= std::max(point1.x(), point2.x()); ++x)
	//		{
	//			floor[point1.y()][x] = '#';
	//		}
	//	} 
	//	else
	//	{
	//		std::cerr << "Non-straight line between points ID " << point1.get_id() << " and ID " << point2.get_id() << std::endl;
	//	}
	//}
	//tools::print_charmatrix(floor);
	//std::cout << "\n\nScaled view:\n";
	//tools::print_charmatrix_scaled(floor);
	
	// Bestimme inneren Startpunkt für Flood-Fill
	//if (point64_ts.size() >= 3)
	//{
	//	const auto& p1 = point64_ts[0];
	//	const auto& p2 = point64_ts[1];
	//	const auto& p3 = point64_ts[2];
	//	
	//	// Richtung von p1 zu p2
	//	int64_t dir1_x = (p2.x() > p1.x()) ? 1 : (p2.x() < p1.x()) ? -1 : 0;
	//	int64_t dir1_y = (p2.y() > p1.y()) ? 1 : (p2.y() < p1.y()) ? -1 : 0;
	//	
	//	// Richtung von p2 zu p3
	//	int64_t dir2_x = (p3.x() > p2.x()) ? 1 : (p3.x() < p2.x()) ? -1 : 0;
	//	int64_t dir2_y = (p3.y() > p2.y()) ? 1 : (p3.y() < p2.y()) ? -1 : 0;
	//	
	//	// Startpunkt: p1 + dir1 + dir2
	//	Point64_t2d inner_start;
	//	inner_start.x() = p1.x() + dir1_x + dir2_x;
	//	inner_start.y() = p1.y() + dir1_y + dir2_y;
	//	
	//	std::cout << "Inner start point: (" << inner_start.x() << ", " << inner_start.y() << ")" << std::endl;
	//	
	//	// Fülle den inneren Bereich
	//	fill_area_in_cahrmatrix(floor, inner_start);
	//	
	//	/*std::cout << "\nAfter filling:\n";
	//	tools::print_charmatrix(floor);*/
	//	
	//	// Finde das größte Rechteck, das vollständig mit # gefüllt ist
	//	int64_t max_filled_area = 0;
	//	PairWithArea max_filled_pair{0, 0, 0};
	//	
	//	for (const auto& pair : all_pairs)
	//	{
	//		auto it1 = std::ranges::find(point64_ts, pair.id1, &Point64_t2d::get_id);
	//		auto it2 = std::ranges::find(point64_ts, pair.id2, &Point64_t2d::get_id);
	//		
	//		if (it1 != point64_ts.end() && it2 != point64_ts.end())
	//		{
	//			if (is_rectangle_filled(floor, *it1, *it2))
	//			{
	//				max_filled_area = pair.area;
	//				max_filled_pair = pair;
	//				break;
	//			}
	//		}
	//	}
	//	
	//	std::cout << "\nMax filled area: " << max_filled_area << std::endl;
	//	std::cout << "Between points ID " << max_filled_pair.id1 << " and ID " << max_filled_pair.id2 << std::endl;
	//	
	//	if (max_filled_area > 0)
	//	{
	//		auto it1 = std::ranges::find(point64_ts, max_filled_pair.id1, &Point64_t2d::get_id);
	//		auto it2 = std::ranges::find(point64_ts, max_filled_pair.id2, &Point64_t2d::get_id);
	//		std::cout << "Point 1 coordinates: (" << it1->x() << ", " << it1->y() << ")" << std::endl;
	//		std::cout << "Point 2 coordinates: (" << it2->x() << ", " << it2->y() << ")" << std::endl;
	//	}
	//}

	return 0;
}

