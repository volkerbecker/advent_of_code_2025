#pragma once

#include <string>
#include <vector>
#include "matrix.h"

struct Shape {
    int id{};
    tools::CharMatrix shape{};
    std::vector<tools::CharMatrix> derived_shapes{};
    void build_derived_shapes();
};

struct Region {
    size_t width{};
    size_t height{};
    std::vector<int> shapes_to_integrate{};
};

void load_input_file(const std::string& filepath, std::vector<Shape>& shapes, std::vector<Region>& regions);
